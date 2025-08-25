<script lang="ts" setup>
import { ref, defineProps, watch } from 'vue'
import { type IntroductionData } from '@/components/introduction/Introduction.vue'
import Introduction from '@/components/introduction/Introduction.vue'
import {
  GameStateType,
  MiniGameIntroductionPayload,
  type MiniGamePayloadType
} from '@/flatbuffers/messageClass'
import {
  type MarbleManiaData,
  type MarbleManiaResult,
  type MarbleManiaEntity,
  type MMVec2
} from './MarbleManiaModels'
import {
  parseMarbleManiaHostPayload,
  parseMarbleManiaResultPayload
} from './MarbleManiaProcessor'
import { Application } from 'vue3-pixi'
import { Graphics, Sprite, Text, TextStyle, CanvasTextMetrics } from 'pixi.js'

defineProps<{ width: number; height: number }>()

enum ViewState { None, Introduction, MiniGame, Results }
const viewState = ref<ViewState>(ViewState.None)

// introduction
const intro = ref<IntroductionData>({ title: '', description: '', time_left: 0 })

// host payload data
const payloadData = ref<MarbleManiaData>({
  entities: [],
  finish_line_y: 0,
  game_phase: 0,
  placement_time_left: 0,
  world_min: { x: -400, y: -300 },
  world_max: { x: 400, y: 400 }
})

// final results
const results = ref<MarbleManiaResult>({ results: [] })

const update = (data: MiniGamePayloadType) => {
  switch (data.gamestatetype()) {
    case GameStateType.MarbleManiaHost: {
      viewState.value = ViewState.MiniGame
      payloadData.value = parseMarbleManiaHostPayload(data)
      break
    }
    case GameStateType.MiniGameIntroduction: {
      viewState.value = ViewState.Introduction
      const introPayload: MiniGameIntroductionPayload = data.gamestatepayload(
        new MiniGameIntroductionPayload()
      )
      intro.value = {
        title: introPayload.name() || '',
        description: introPayload.instruction() || '',
        time_left: Number(introPayload.timeLeft())
      }
      break
    }
    case GameStateType.MarbleManiaResult: {
      viewState.value = ViewState.Results
      results.value = parseMarbleManiaResultPayload(data)
      break
    }
  }
  return []
}

// ---------- Rendering helpers (center-based) ----------

const CANVAS_W = 800
const CANVAS_H = 600
const PAD = 10 // border padding

const worldToScreen = (w: MMVec2) => {
  const wm = payloadData.value.world_min
  const wx = payloadData.value.world_max
  const worldW = wx.x - wm.x
  const worldH = wx.y - wm.y
  const x = (w.x - wm.x) * (CANVAS_W - 2 * PAD) / worldW + PAD
  const y = (w.y - wm.y) * (CANVAS_H - 2 * PAD) / worldH + PAD
  return { x, y }
}

const getEntityCenter = (e: MarbleManiaEntity) => worldToScreen(e.pos)

const rotateLocal = (v: MMVec2, rot: number): MMVec2 => {
  const c = Math.cos(rot), s = Math.sin(rot)
  return { x: v.x * c - v.y * s, y: v.x * s + v.y * c }
}

const add = (a: MMVec2, b: MMVec2): MMVec2 => ({ x: a.x + b.x, y: a.y + b.y })

// ---------- Drawing primitives ----------

const renderMarble = (g: Graphics, e: MarbleManiaEntity) => {
  if (e.shape.kind !== 'circle') return // marbles are circles
  const center = getEntityCenter(e)
  const radiusWorld = e.shape.radius

  // Convert radius from world to screen: scale x/y separately, take average to keep it circular
  const rX = Math.abs(worldToScreen({ x: e.pos.x + radiusWorld, y: e.pos.y }).x - center.x)
  const rY = Math.abs(worldToScreen({ x: e.pos.x, y: e.pos.y + radiusWorld }).y - center.y)
  const r = (rX + rY) * 0.5

  g.lineStyle(2, 0x2d5016)
  g.beginFill(0x4ade80)
  g.drawCircle(center.x, center.y, r)
  g.endFill()

  // little highlight
  g.lineStyle(0)
  g.beginFill(0x86efac, 0.6)
  g.drawCircle(center.x - r * 0.3, center.y - r * 0.3, r * 0.3)
  g.endFill()
}

const renderCircleObstacle = (g: Graphics, e: MarbleManiaEntity) => {
  if (e.shape.kind !== 'circle') return
  const center = getEntityCenter(e)
  const rX = Math.abs(worldToScreen({ x: e.pos.x + e.shape.radius, y: e.pos.y }).x - center.x)
  const rY = Math.abs(worldToScreen({ x: e.pos.x, y: e.pos.y + e.shape.radius }).y - center.y)
  const r = (rX + rY) * 0.5

  g.lineStyle(2, 0x333333)
  g.beginFill(0x8b4513)
  g.drawCircle(center.x, center.y, r)
  g.endFill()
}

const renderRectObstacle = (g: Graphics, e: MarbleManiaEntity) => {
  if (e.shape.kind !== 'rect') return
  const hw = e.shape.width / 2
  const hh = e.shape.height / 2

  // local corners around center
  const corners: MMVec2[] = [
    { x: -hw, y: -hh },
    { x: +hw, y: -hh },
    { x: +hw, y: +hh },
    { x: -hw, y: +hh }
  ]
  // rotate + translate to world, then map to screen
  const pts = corners.map(c => {
    const world = add(e.pos, rotateLocal(c, e.rotation || 0))
    return worldToScreen(world)
  })

  g.lineStyle(2, 0x333333)
  g.beginFill(0x8b4513)
  g.drawPolygon(pts.flatMap(p => [p.x, p.y]))
  g.endFill()
}

const renderPolyObstacle = (g: Graphics, e: MarbleManiaEntity) => {
  if (e.shape.kind !== 'poly') return
  const pts = e.shape.vertices.map(v => {
    const world = add(e.pos, rotateLocal(v, e.rotation || 0))
    return worldToScreen(world)
  })

  g.lineStyle(2, 0x333333)
  g.beginFill(0xb8860b)
  g.drawPolygon(pts.flatMap(p => [p.x, p.y]))
  g.endFill()
}

// ---------- Render loops ----------

const render = (g: Graphics) => {
  g.clear()

  // frame
  g.lineStyle(4, 0xffffff)
  g.drawRect(PAD, PAD, CANVAS_W - 2 * PAD, CANVAS_H - 2 * PAD)

  // drop zone (top band)
  if (payloadData.value.game_phase === 0) {
    g.lineStyle(2, 0x00ff00)
    g.beginFill(0x00ff00, 0.1)
    g.drawRect(PAD + 50, PAD + 10, CANVAS_W - 2 * (PAD + 50), 100)
    g.endFill()
  }

  // finish line
  const fy = worldToScreen({ x: 0, y: payloadData.value.finish_line_y }).y
  g.lineStyle(4, 0xff0000)
  g.moveTo(PAD, fy)
  g.lineTo(CANVAS_W - PAD, fy)
  g.beginFill(0xff0000)
  for (let x = PAD; x < CANVAS_W - PAD; x += 20) {
    if (Math.floor((x - PAD) / 20) % 2 === 0) {
      g.drawRect(x, fy - 5, 20, 10)
    }
  }
  g.endFill()

  // placement timer
  if (payloadData.value.game_phase === 0 && payloadData.value.placement_time_left > 0) {
    const timerW = 200, timerH = 20, timerX = CANVAS_W / 2 - timerW / 2, timerY = PAD + 120
    g.lineStyle(2, 0xffffff)
    g.beginFill(0x333333)
    g.drawRect(timerX, timerY, timerW, timerH)
    g.endFill()
    const progress = Math.max(0, payloadData.value.placement_time_left / 15)
    g.beginFill(progress > 0.3 ? 0x00ff00 : 0xff0000)
    g.drawRect(timerX + 2, timerY + 2, (timerW - 4) * progress, timerH - 4)
    g.endFill()
  }

  // obstacles
  for (const e of getObstacles()) {
    if (e.shape.kind === 'circle') renderCircleObstacle(g, e)
    else if (e.shape.kind === 'rect') renderRectObstacle(g, e)
    else renderPolyObstacle(g, e)
  }

  // marbles
  for (const e of getMarbles()) {
    renderMarble(g, e)
  }
}

const renderResults = (g: Graphics) => {
  g.clear()
  g.lineStyle(2, 0x333333)
  g.beginFill(0x1a1a1a, 0.8)
  g.drawRect(0, 0, CANVAS_W, CANVAS_H)
  g.endFill()

  g.lineStyle(3, 0xffd700)
  g.beginFill(0x333333, 0.9)
  g.drawRoundedRect(CANVAS_W / 2 - 200, 50, 400, 60, 10)
  g.endFill()
}

// utils
const getMarbles = () => payloadData.value.entities.filter(e => e.type === 'marble')
const getObstacles = () => payloadData.value.entities.filter(e => e.type === 'obstacle')

const applicationId = ref(0)
watch(
  () => payloadData.value.entities,
  (n, o) => { if (o && n && n.length !== o.length) applicationId.value++ },
  { immediate: true }
)

const textStyle = new TextStyle({
  fontFamily: ['Helvetica', 'Arial', 'sans-serif'],
  fontSize: 14,
  fill: 'white',
  stroke: 'black',
  // @ts-expect-error: PixiJS typing quirk
  strokeThickness: 2
})

const getCenteredTextPosition = (text: string, x: number): number => {
  const metrics = CanvasTextMetrics.measureText(text, textStyle)
  return x - metrics.width / 2
}

const pr = new Intl.PluralRules('en-US', { type: 'ordinal' })
const suffixes = new Map([
  ['one', 'st'],
  ['two', 'nd'],
  ['few', 'rd'],
  ['other', 'th']
])
const formatOrdinals = (n: number) => `${n}${suffixes.get(pr.select(n))}`

defineExpose({ update })
</script>

<template>
  <div class="h-full">
    <div v-if="viewState == ViewState.Introduction">
      <Introduction :data="intro" logoSVG="/assets/games/MarbleMania/MarbleManiaLogo.svg" />
    </div>

    <div v-else-if="viewState == ViewState.MiniGame">
      <div class="flex flex-col h-full w-full justify-center items-center">
        <div class="text-4xl m-6">
          <span v-if="payloadData.game_phase === 0">
            Place Your Marbles! Time: {{ Math.ceil(payloadData.placement_time_left) }}s
          </span>
          <span v-else-if="payloadData.game_phase === 1">Marbles are Rolling!</span>
          <span v-else>Simulation Complete</span>
        </div>

        <div class="absolute top-0 text text-white z-10">
          <div class="flex flex-col justify-center items-center mt-32">
            <div v-if="payloadData.game_phase === 0" class="text-3xl flex flex-col justify-center items-center">
              <div>Drop your marbles in the green zone!</div>
            </div>
          </div>
        </div>

        <div class="relative">
          <div class="absolute top-0 left-0 w-full h-full bg-black"></div>
          <div class="relative">
            <Application :key="applicationId" :width="800" :height="600" background-color="black">
              <Graphics :x="0" :y="0" @render="render" />
              <!-- marble labels & finished icons -->
              <template v-for="marble in getMarbles()" :key="marble.id">
                <Text
                  :position="{
                    x: getCenteredTextPosition(marble.player_name || marble.id, getEntityCenter(marble).x),
                    y: getEntityCenter(marble).y - 24
                  }"
                  :text="marble.player_name || marble.id"
                  :style="{ fontFamily: ['Helvetica','Arial','sans-serif'], fontSize: 18, fill: 'white', stroke: 'black', strokeThickness: 4 }"
                />
                <Sprite
                  v-if="marble.finished"
                  :position="{ x: getEntityCenter(marble).x - 20, y: getEntityCenter(marble).y - 20 }"
                  :width="40"
                  :height="40"
                  texture="/assets/games/marbleMania/checkmark.png"
                />
              </template>
            </Application>
          </div>
        </div>
      </div>
    </div>

    <div v-else-if="viewState == ViewState.Results" class="flex flex-col h-full w-full justify-center items-center">
      <div class="text-4xl m-6">Marble Mania Results</div>
      <div class="relative">
        <div class="absolute top-0 left-0 w-full h-full bg-black"></div>
        <div class="relative">
          <Application :width="800" :height="600" background-color="black">
            <Graphics :x="0" :y="0" @render="renderResults" />
            <template v-for="(result, idx) in results.results" :key="result.name">
              <Text
                :position="{ x: getCenteredTextPosition(`${formatOrdinals(result.placement)}. ${result.name}`, 400), y: 150 + idx * 60 }"
                :text="`${formatOrdinals(result.placement)}. ${result.name}`"
                :style="{ fontFamily: ['Helvetica','Arial','sans-serif'], fontSize: 24, fill: result.has_finished ? 'white' : 'gray', stroke: 'black', strokeThickness: 3 }"
              />
              <Text
                v-if="result.has_finished"
                :position="{ x: getCenteredTextPosition(`Time: ${result.time_to_finish.toFixed(2)}s`, 400), y: 175 + idx * 60 }"
                :text="`Time: ${result.time_to_finish.toFixed(2)}s`"
                :style="{ fontFamily: ['Helvetica','Arial','sans-serif'], fontSize: 16, fill: 'lightblue', stroke: 'black', strokeThickness: 2 }"
              />
              <Text
                v-else
                :position="{ x: getCenteredTextPosition('Did not finish', 400), y: 175 + idx * 60 }"
                text="Did not finish"
                :style="{ fontFamily: ['Helvetica','Arial','sans-serif'], fontSize: 16, fill: 'red', stroke: 'black', strokeThickness: 2 }"
              />
            </template>
          </Application>
        </div>
      </div>
    </div>
  </div>
</template>

<style scoped>
</style>
