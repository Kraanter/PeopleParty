<script lang="ts" setup>
import { ref, computed, watch } from 'vue'
import { Container, Graphics, Sprite, Text, TextStyle, CanvasTextMetrics } from 'pixi.js'
import { useGameCanvas, createEntityLayer, useSnapshotBuffer } from '@/composables/pixi'
import type { MarbleManiaData, MarbleManiaEntity, MMVec2 } from './MarbleManiaModels'

const props = defineProps<{
  hostData: MarbleManiaData | null
  getPlayerColor: (playerId: string) => number
}>()

// Fixed logical camera viewport (not the game world, which is much taller and scrolls past
// it) — useGameCanvas letterboxes this into whatever space is actually available, replacing
// the previous window.innerHeight-based sizing (which also squished the canvas square below
// 600px of available height, a latent bug fixed as a side effect of correct letterboxing).
const VIEWPORT_WIDTH = 800
const VIEWPORT_HEIGHT = 600
const PAD = 10

// Toggle to compare against instant-snap marble positioning while debugging. Marble world
// positions only change once per network tick (~50ms); rendering them raw makes a marble hold
// still for ~3 frames then jump (visible as a stutter now that the camera moves continuously
// alongside it), while renders in between are exact — every result is one of two mutually
// exclusive tradeoffs, never a compromise. When true, positions are buffered-interpolated
// instead (see marbleSamples/interpolatedPos below) — smooth, and bounded to the range spanned
// by two real received positions, unlike an ease that chases a moving target and can visibly
// overshoot/round off a sharp bounce. Flip to false to see the raw, unsmoothed positions.
const MARBLE_SMOOTHING = true

const marbleLabelStyle = new TextStyle({
  fontFamily: ['Helvetica', 'Arial', 'sans-serif'],
  fontSize: 18,
  fill: 'white',
  stroke: { color: 'black', width: 4 }
})

const offScreenTextStyle = new TextStyle({
  fontFamily: ['Arial', 'sans-serif'],
  fontSize: 20,
  fill: 'white',
  stroke: { color: 'black', width: 3 },
  align: 'center'
})

class MarbleDisplay extends Container {
  body = new Graphics()
  nameText = new Text({ text: '', style: marbleLabelStyle })
  checkmark = Sprite.from('/assets/games/marbleMania/checkmark.png')

  constructor() {
    super()
    this.checkmark.width = 40
    this.checkmark.height = 40
    this.addChild(this.body, this.nameText, this.checkmark)
  }
}

class ObstacleDisplay extends Graphics {
  drawn = false
}

class OffScreenDisplay extends Container {
  dot = new Graphics()
  text = new Text({ text: '', style: offScreenTextStyle })

  constructor() {
    super()
    this.text.anchor.set(0.5)
    this.addChild(this.dot, this.text)
  }
}

function darken(color: number): number {
  return (
    Math.floor((color & 0xff) * 0.6) |
    (Math.floor(((color >> 8) & 0xff) * 0.6) << 8) |
    (Math.floor(((color >> 16) & 0xff) * 0.6) << 16)
  )
}

function lighten(color: number): number {
  return (
    Math.min(255, Math.floor((color & 0xff) * 1.3)) |
    (Math.min(255, Math.floor(((color >> 8) & 0xff) * 1.3)) << 8) |
    (Math.min(255, Math.floor(((color >> 16) & 0xff) * 1.3)) << 16)
  )
}

function getCenteredTextPosition(text: string, x: number, style: TextStyle): number {
  const metrics = CanvasTextMetrics.measureText(text, style)
  return x - metrics.width / 2
}

function rotateLocal(v: MMVec2, rot: number): MMVec2 {
  const c = Math.cos(rot)
  const s = Math.sin(rot)
  return { x: v.x * c - v.y * s, y: v.x * s + v.y * c }
}

// ─── Canvas + entity layers ───────────────────────────────────────────────────

const containerRef = ref<HTMLElement | null>(null)

const canvas = useGameCanvas({
  container: containerRef,
  worldSize: { width: VIEWPORT_WIDTH, height: VIEWPORT_HEIGHT },
  backgroundColor: 0x000000,
  render: () => drawScene()
})

const worldGfx = new Graphics()
canvas.root.addChild(worldGfx)

const obstaclesLayerContainer = new Container()
canvas.root.addChild(obstaclesLayerContainer)
const marblesLayerContainer = new Container()
canvas.root.addChild(marblesLayerContainer)
const offScreenLayerContainer = new Container()
canvas.root.addChild(offScreenLayerContainer)

const obstacleLayer = createEntityLayer<MarbleManiaEntity, ObstacleDisplay>(
  obstaclesLayerContainer,
  {
    key: (e) => e.id,
    create: () => new ObstacleDisplay(),
    update: (display, entity) => {
      display.visible = isEntityInCameraView(entity)
      if (!display.drawn) {
        drawObstacleShape(display, entity)
        display.drawn = true
      }
      if (!display.visible) return
      const center = worldToScreen(entity.pos)
      display.position.set(center.x, center.y)
    }
  }
)

const marbleLayer = createEntityLayer<MarbleManiaEntity, MarbleDisplay>(marblesLayerContainer, {
  key: (e) => e.id,
  create: () => new MarbleDisplay(),
  update: (display, entity) => {
    // Culling itself stays on the real (unsmoothed) position — using a lagging smoothed
    // position here could pop a marble in/out at the camera boundary a frame early/late.
    display.visible = isEntityInCameraView(entity)
    if (!display.visible) return
    const center = worldToScreen(interpolatedPos(entity))
    display.position.set(center.x, center.y)
    drawMarbleBody(display, entity)

    const playerId = entity.player_name || entity.id
    display.nameText.text = playerId
    display.nameText.position.set(getCenteredTextPosition(playerId, 0, marbleLabelStyle), -24)
    display.checkmark.visible = entity.finished
    display.checkmark.position.set(-20, -20)
  }
})

const offScreenIndicatorLayer = createEntityLayer<MarbleManiaEntity, OffScreenDisplay>(
  offScreenLayerContainer,
  {
    key: (e) => e.id,
    create: () => new OffScreenDisplay(),
    update: (display, entity) => {
      const screenX = worldToScreen({ x: entity.pos.x, y: cameraOffset.y }).x
      const distance = Math.round((cameraOffset.y - entity.pos.y) / 5)
      const playerId = entity.player_name || entity.id
      const color = props.getPlayerColor(playerId)
      const border = darken(color)

      const dotY = 20
      const textY = dotY + 18
      const label = `${distance}`
      const textWidth = label.length * 8
      const textHeight = 14

      display.dot.clear()
      display.dot.circle(screenX, dotY, 6).fill(color).stroke({ width: 2, color: border })
      display.dot
        .rect(
          screenX - textWidth / 2 - 2,
          textY - textHeight / 2 - 1,
          textWidth + 4,
          textHeight + 2
        )
        .fill({ color: 0x000000, alpha: 0.8 })

      display.text.text = label
      display.text.position.set(screenX, textY)
    }
  }
)

// ─── Snapshot state ────────────────────────────────────────────────────────────

interface MarbleManiaHud {
  game_phase: number
  placement_time_left: number
}

const buffer = useSnapshotBuffer<MarbleManiaData, MarbleManiaHud>(
  {
    entities: [],
    game_phase: 0,
    placement_time_left: 0,
    finish_line_y: 0,
    world_min: { x: -400, y: -300 },
    world_max: { x: 400, y: 400 }
  },
  {
    invalidate: canvas.invalidate,
    toHud: (d) => ({ game_phase: d.game_phase, placement_time_left: d.placement_time_left })
  }
)
const hud = computed(() => buffer.hud.value!)

const getMarbles = () => buffer.current.entities.filter((e) => e.type === 'marble')
const getObstacles = () => buffer.current.entities.filter((e) => e.type === 'obstacle')
const getOffScreenMarbles = () => getMarbles().filter((m) => m.pos.y < cameraOffset.y)

// Camera — imperative, non-reactive state; nothing in the template reads it directly.
let cameraOffset = { x: 0, y: 0 }
let smoothedCameraY = 0
// Gates the single shared per-frame callback (camera-follow + marble smoothing together).
let isAnimating = false

// scale is fixed (depends only on world bounds + the viewport constants, never on
// cameraOffset), so it's safe to bake into a shape's path once and only reposition afterward.
function worldScale(): MMVec2 {
  const worldWidth = buffer.current.world_max.x - buffer.current.world_min.x
  const cameraWorldHeight = (VIEWPORT_HEIGHT / VIEWPORT_WIDTH) * worldWidth
  return {
    x: (VIEWPORT_WIDTH - 2 * PAD) / worldWidth,
    y: (VIEWPORT_HEIGHT - 2 * PAD) / cameraWorldHeight
  }
}

function worldToScreen(w: MMVec2): MMVec2 {
  const s = worldScale()
  return {
    x: (w.x - cameraOffset.x) * s.x + PAD,
    y: (w.y - cameraOffset.y) * s.y + PAD
  }
}

function isEntityInCameraView(entity: MarbleManiaEntity): boolean {
  const worldWidth = buffer.current.world_max.x - buffer.current.world_min.x
  const cameraWorldHeight = (VIEWPORT_HEIGHT / VIEWPORT_WIDTH) * worldWidth
  const cameraMinX = cameraOffset.x
  const cameraMaxX = cameraOffset.x + worldWidth
  const cameraMinY = cameraOffset.y
  const cameraMaxY = cameraOffset.y + cameraWorldHeight

  let entityMinX = entity.pos.x
  let entityMaxX = entity.pos.x
  let entityMinY = entity.pos.y
  let entityMaxY = entity.pos.y

  if (entity.shape.kind === 'circle') {
    const radius = entity.shape.radius
    entityMinX -= radius
    entityMaxX += radius
    entityMinY -= radius
    entityMaxY += radius
  } else if (entity.shape.kind === 'rect') {
    const hw = entity.shape.width / 2
    const hh = entity.shape.height / 2
    const maxExtent = Math.max(hw, hh) * 1.5
    entityMinX -= maxExtent
    entityMaxX += maxExtent
    entityMinY -= maxExtent
    entityMaxY += maxExtent
  } else if (entity.shape.kind === 'poly') {
    let maxExtent = 50
    for (const vertex of entity.shape.vertices) {
      maxExtent = Math.max(maxExtent, Math.sqrt(vertex.x * vertex.x + vertex.y * vertex.y))
    }
    entityMinX -= maxExtent
    entityMaxX += maxExtent
    entityMinY -= maxExtent
    entityMaxY += maxExtent
  }

  const margin = 50
  entityMinX -= margin
  entityMaxX += margin
  entityMinY -= margin
  entityMaxY += margin

  return !(
    entityMaxX < cameraMinX ||
    entityMinX > cameraMaxX ||
    entityMaxY < cameraMinY ||
    entityMinY > cameraMaxY
  )
}

function drawObstacleShape(display: ObstacleDisplay, entity: MarbleManiaEntity) {
  const shape = entity.shape
  const s = worldScale()
  if (shape.kind === 'circle') {
    const r = shape.radius * (s.x + s.y) * 0.5
    display.circle(0, 0, r).fill(0x8b4513).stroke({ width: 2, color: 0x333333 })
  } else if (shape.kind === 'rect') {
    const hw = shape.width / 2
    const hh = shape.height / 2
    const corners: MMVec2[] = [
      { x: -hw, y: -hh },
      { x: hw, y: -hh },
      { x: hw, y: hh },
      { x: -hw, y: hh }
    ]
    const pts = corners.map((c) => {
      const r = rotateLocal(c, entity.rotation || 0)
      return { x: r.x * s.x, y: r.y * s.y }
    })
    display.poly(pts).fill(0x8b4513).stroke({ width: 2, color: 0x333333 })
  } else {
    const pts = shape.vertices.map((v) => {
      const r = rotateLocal(v, entity.rotation || 0)
      return { x: r.x * s.x, y: r.y * s.y }
    })
    display.poly(pts).fill(0xb8860b).stroke({ width: 2, color: 0x333333 })
  }
}

function drawMarbleBody(display: MarbleDisplay, entity: MarbleManiaEntity) {
  if (entity.shape.kind !== 'circle') return
  const s = worldScale()
  const r = entity.shape.radius * (s.x + s.y) * 0.5
  const playerId = entity.player_name || entity.id
  const color = props.getPlayerColor(playerId)
  const border = darken(color)
  const highlight = lighten(color)

  display.body.clear()
  display.body.circle(0, 0, r).fill(color).stroke({ width: 2, color: border })
  display.body.circle(-r * 0.3, -r * 0.3, r * 0.3).fill({ color: highlight, alpha: 0.6 })
}

function drawScene() {
  const data = buffer.current
  worldGfx.clear()

  const leftBoundary = worldToScreen({ x: data.world_min.x, y: 0 })
  const rightBoundary = worldToScreen({ x: data.world_max.x, y: 0 })
  worldGfx.moveTo(leftBoundary.x, 0).lineTo(leftBoundary.x, VIEWPORT_HEIGHT).stroke({
    width: 4,
    color: 0xffffff
  })
  worldGfx.moveTo(rightBoundary.x, 0).lineTo(rightBoundary.x, VIEWPORT_HEIGHT).stroke({
    width: 4,
    color: 0xffffff
  })

  if (data.game_phase === 0) {
    const dropZoneLeft = worldToScreen({ x: data.world_min.x + 10, y: data.world_min.y + 20 })
    const dropZoneRight = worldToScreen({ x: data.world_max.x - 10, y: data.world_min.y + 120 })
    worldGfx
      .rect(
        dropZoneLeft.x,
        dropZoneLeft.y,
        dropZoneRight.x - dropZoneLeft.x,
        dropZoneRight.y - dropZoneLeft.y
      )
      .fill({ color: 0x00ff00, alpha: 0.1 })
      .stroke({ width: 2, color: 0x00ff00 })
  }

  const fy = worldToScreen({ x: 0, y: data.finish_line_y }).y
  const finishLeft = worldToScreen({ x: data.world_min.x, y: data.finish_line_y })
  const finishRight = worldToScreen({ x: data.world_max.x, y: data.finish_line_y })
  worldGfx.moveTo(finishLeft.x, fy).lineTo(finishRight.x, fy).stroke({ width: 4, color: 0xff0000 })
  for (let x = finishLeft.x; x < finishRight.x; x += 20) {
    if (Math.floor((x - finishLeft.x) / 20) % 2 === 0) {
      worldGfx.rect(x, fy - 5, 20, 10).fill(0xff0000)
    }
  }

  if (data.game_phase === 0 && data.placement_time_left > 0) {
    const timerW = 200
    const timerH = 20
    const timerX = VIEWPORT_WIDTH / 2 - timerW / 2
    const timerY = PAD + 120
    worldGfx
      .rect(timerX, timerY, timerW, timerH)
      .fill(0x333333)
      .stroke({ width: 2, color: 0xffffff })
    const progress = Math.max(0, data.placement_time_left / 15)
    worldGfx
      .rect(timerX + 2, timerY + 2, (timerW - 4) * progress, timerH - 4)
      .fill(progress > 0.3 ? 0x00ff00 : 0xff0000)
  }

  obstacleLayer.sync(getObstacles())
  marbleLayer.sync(getMarbles())
  offScreenIndicatorLayer.sync(getOffScreenMarbles())
}

// ─── Camera follow — real per-frame dt interpolation ──────────────────────────

function computeTargetCameraTop(): number {
  const marbles = getMarbles()
  if (marbles.length === 0) return buffer.current.world_min.y

  // Filter finished marbles out BEFORE reducing, not just during — a no-seed reduce starts
  // from marbles[0] unconditionally, so a `!marble.finished` check only inside the comparator
  // can never dethrone an already-finished marbles[0]: nothing still racing can exceed the
  // position of whoever was in the lead when they finished. Filtering first fixes this for
  // any array order, not just index 0. When EVERYONE has finished (the ~2s tail at the end of
  // every race, once game_phase hits 2), fall back to the full set — deliberately without the
  // finished guard, since filtering already decided the candidate set.
  const active = marbles.filter((m) => !m.finished)
  const candidates = active.length > 0 ? active : marbles
  const lowestMarble = candidates.reduce((lowest, marble) =>
    marble.pos.y > lowest.pos.y ? marble : lowest
  )
  const worldWidth = buffer.current.world_max.x - buffer.current.world_min.x
  const cameraWorldHeight = (VIEWPORT_HEIGHT / VIEWPORT_WIDTH) * worldWidth

  let targetTop = lowestMarble.pos.y - cameraWorldHeight * 0.66
  const maxTop = buffer.current.world_max.y - cameraWorldHeight
  const minTop = buffer.current.world_min.y
  return Math.max(minTop, Math.min(maxTop, targetTop))
}

// Backend's marbleMania target_fps=20 → 1000/20=50ms host-update send interval. The 0.25
// convergence factor below was tuned (pre-migration) for "per network tick", not "per rendered
// frame" — using the display frame period here made the camera converge ~3x faster than
// intended (snap almost immediately to each new target, then sit idle until the next arrives).
const NETWORK_TICK_MS = 50

function animateCamera({ dt }: { dt: number }) {
  const target = computeTargetCameraTop()
  // Frame-rate-independent equivalent of "25% of the remaining distance per ~50ms network tick".
  const easeFactor = 1 - Math.pow(1 - 0.25, dt / NETWORK_TICK_MS)
  smoothedCameraY += (target - smoothedCameraY) * easeFactor
  // x always tracks world_min.x fresh ("always show full width") rather than carrying
  // forward whatever x happened to be — matches the original updateCamera()'s unconditional
  // `cameraOffset.value.x = payloadData.value.world_min.x` on every call.
  cameraOffset = { x: buffer.current.world_min.x, y: smoothedCameraY }
}

interface MarbleSample {
  pos: MMVec2
  t: number // performance.now() at client receipt — no backend/schema changes needed; WebSocket
  // delivery is ordered, so t is monotonic by construction (each list stays sorted ascending)
}

const marbleSamples = new Map<string, MarbleSample[]>() // last MAX_SAMPLES real samples per marble

// A 2-sample buffer forces a very narrow, unforgiving choice of delay: for regularly-spaced
// ticks every P ms, full-window (zero-clamping) coverage only happens in the single-point range
// D=P exactly — anywhere else, part or all of every window clamps to one endpoint instead of
// truly interpolating. D=2P (the previous value here) isn't "extra-safe margin," it's the exact
// point where that coverage window hits zero — every render clamps to the older sample for the
// whole ~50ms gap, then jumps once, which is a discrete step function in disguise, not smoothing.
// Real jitter pushes the effective local delay back and forth across that cliff edge frame to
// frame, which reads as jitter rather than a clean stagger. Buffering more samples turns this
// single point into a real range — for N samples, full coverage holds for any D in [P, (N-1)P] —
// so MAX_SAMPLES=5 (packet-loss/delayed-tick headroom) with INTERP_DELAY_MS=1.5P sits with margin
// on both sides of that range instead of exactly on its boundary.
const MAX_SAMPLES = 5
const INTERP_DELAY_MS = NETWORK_TICK_MS * 1.5 // ~75ms

function recordMarbleSamples() {
  const now = performance.now()
  for (const marble of getMarbles()) {
    const list = marbleSamples.get(marble.id) ?? []
    list.push({ pos: { ...marble.pos }, t: now })
    if (list.length > MAX_SAMPLES) list.shift()
    marbleSamples.set(marble.id, list)
  }
}

// Interpolates between two of the most recent REAL received positions rather than easing toward
// whichever is latest — the latter always lags a moving target by an amount proportional to
// speed, and during a bounce (a step-discontinuity in velocity) that lag makes the display
// visibly "float" past where the true, physics-authoritative position already turned — a sharp
// corner low-pass-filtered always looks rounded, no matter the convergence rate. This is bounded
// by construction instead: every branch below returns a real sample or a point ON the segment
// between two consecutive real samples — it can never leave the range they span, which is
// exactly what rules the floating-above-the-obstacle artifact out rather than just shrinking it.
// Searches for the bracketing pair rather than assuming it's always samples[0]/[1] — which pair
// actually brackets renderTime shifts around with real arrival jitter, not just a fixed offset.
function interpolatedPos(entity: MarbleManiaEntity): MMVec2 {
  if (!MARBLE_SMOOTHING) return entity.pos
  const samples = marbleSamples.get(entity.id)
  if (!samples || samples.length === 0) return entity.pos
  if (samples.length === 1) return samples[0].pos

  const renderTime = performance.now() - INTERP_DELAY_MS
  if (renderTime <= samples[0].t) return samples[0].pos
  for (let i = 0; i < samples.length - 1; i++) {
    const older = samples[i]
    const newer = samples[i + 1]
    if (renderTime >= older.t && renderTime <= newer.t) {
      const frac = newer.t > older.t ? (renderTime - older.t) / (newer.t - older.t) : 0
      return {
        x: older.pos.x + (newer.pos.x - older.pos.x) * frac,
        y: older.pos.y + (newer.pos.y - older.pos.y) * frac
      }
    }
  }
  return samples[samples.length - 1].pos // past every sample — hold, never extrapolate
}

// Union by id rather than only patching entities already known: the backend sends obstacles
// in the entities array exactly once ever (never re-sent unless the round is paused/resumed),
// so any merge that only updates existing entries — never adding ones it hasn't seen before —
// permanently loses obstacles the moment the message carrying them is missed. This also
// removes the need for a separate "is this the very first snapshot" branch: an empty `prev`
// just means every incoming entity is new, which this handles the same way as any other update.
function mergeEntities(
  prev: MarbleManiaEntity[],
  incoming: MarbleManiaEntity[]
): MarbleManiaEntity[] {
  const byId = new Map(prev.map((e) => [e.id, e] as const))
  for (const e of incoming) byId.set(e.id, e) // backend sends full entities every tick, never a partial diff
  return [...byId.values()]
}

function mergeAndPush(newData: MarbleManiaData) {
  const merged: MarbleManiaData = {
    ...newData,
    entities: mergeEntities(buffer.current.entities, newData.entities)
  }
  buffer.push(merged)

  if (merged.game_phase === 1 || merged.game_phase === 2) {
    recordMarbleSamples()
    if (smoothedCameraY === 0) smoothedCameraY = cameraOffset.y
    if (!isAnimating) {
      canvas.startAnimating(animateCamera)
      isAnimating = true
    }
  } else {
    if (isAnimating) {
      canvas.stopAnimating()
      isAnimating = false
    }
    const topPosition = merged.world_min.y
    cameraOffset = { x: merged.world_min.x, y: topPosition }
    smoothedCameraY = topPosition
    // Placement reset — clear buffered marble samples too, or a marble re-placed far from
    // where it last was mid-race would interpolate a glide there instead of appearing instantly.
    marbleSamples.clear()
  }
}

// A reactive prop, not a push()-via-ref call — HostView's update() sets the prop and flips
// viewState in the same synchronous tick, so the child's own template ref isn't populated yet
// on the very first message. Props don't have that problem: they're already attached at mount
// time, and { immediate: true } below makes sure the watcher itself doesn't miss that first
// value either (a watch() without it only fires on a *subsequent* change).
watch(
  () => props.hostData,
  (data) => {
    if (data) mergeAndPush(data)
  },
  { immediate: true }
)

defineExpose({
  ...(import.meta.env.DEV
    ? {
        __devCameraY: () => smoothedCameraY,
        // A fresh object, not the Map's own stored (mutated-in-place) reference — assigning the
        // same reference back to a Vue ref() every poll doesn't trigger reactivity, since Vue
        // compares by identity and the reference itself never changes even though its fields do.
        __devMarblePos: (id: string) => ({
          ...interpolatedPos(getMarbles().find((m) => m.id === id)!)
        })
      }
    : {})
})
</script>

<template>
  <div class="flex flex-col h-full w-full items-center">
    <div class="text-4xl m-6 text-white shrink-0">
      <span v-if="hud.game_phase === 0">
        Place Your Marbles! Time: {{ Math.ceil(hud.placement_time_left) }}s
      </span>
      <span v-else-if="hud.game_phase === 1">Marbles are Rolling!</span>
      <span v-else>Simulation Complete</span>
    </div>

    <div class="absolute top-0 text text-white z-10">
      <div class="flex flex-col justify-center items-center mt-72">
        <div v-if="hud.game_phase === 0" class="text-3xl flex flex-col justify-center items-center">
          <div>Drop your marbles in the green zone!</div>
        </div>
      </div>
    </div>

    <div class="relative flex-1 w-full min-h-0">
      <div class="absolute top-0 left-0 w-full h-full bg-black"></div>
      <div
        ref="containerRef"
        class="relative w-full h-full flex items-center justify-center overflow-hidden"
      ></div>
    </div>
  </div>
</template>
