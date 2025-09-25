<script lang="ts" setup>
import { ref, defineProps, watch, computed, onMounted, onUnmounted } from 'vue'
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

// Container dimensions
const containerHeight = ref(800) // Will be updated dynamically
const containerRef = ref<HTMLElement>()

// Camera system
const cameraOffset = ref({ x: 0, y: 0 })
const smoothedCameraY = ref(0) // Smoothed camera position for interpolation
const cameraSmoothing = 0.25 // Smoothing factor (0.25 = 25% movement per frame)

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

// Fixed camera viewport dimensions - independent of world size
const canvasWidth = computed(() => {
  // Fixed width - shows full world width (800 units) at a comfortable scale
  return Math.min(800, containerHeight.value - 200) // Cap at reasonable size
})

const canvasHeight = computed(() => {
  // Fixed height for camera viewport - shows only a portion of the world
  const maxHeight = containerHeight.value - 200 // Leave space for UI elements
  return Math.min(600, maxHeight) // Fixed viewport height, independent of world height
})

// Camera system - follows the lowest marble with fixed viewport
const updateCamera = () => {
  const marbles = getMarbles()
  if (marbles.length === 0) {
    // Reset to top during placement or when no marbles
    const targetY = payloadData.value.world_min.y
    smoothedCameraY.value = targetY
    cameraOffset.value.y = targetY
    return
  }
  
  // Find the lowest marble (highest Y value since Y increases downward)
  const lowestMarble = marbles.reduce((lowest, marble) => 
    marble.pos.y > lowest.pos.y && !marble.finished ? marble : lowest
  )
  
  // Calculate camera viewport height in world units
  const worldWidth = payloadData.value.world_max.x - payloadData.value.world_min.x
  
  // How much world space does our fixed camera viewport cover?
  const cameraWorldHeight = (canvasHeight.value / canvasWidth.value) * worldWidth // Maintain aspect ratio

  // Position camera so lowest marble appears at 66% down from the top of viewport
  let targetCameraTop = lowestMarble.pos.y - (cameraWorldHeight * 0.66)
  
  // Clamp camera so it doesn't go beyond world boundaries
  const maxCameraTop = payloadData.value.world_max.y - cameraWorldHeight
  const minCameraTop = payloadData.value.world_min.y
  
  targetCameraTop = Math.max(minCameraTop, Math.min(maxCameraTop, targetCameraTop))
  
  // Smooth camera interpolation - gradually move toward target
  smoothedCameraY.value += (targetCameraTop - smoothedCameraY.value) * cameraSmoothing
  
  cameraOffset.value.y = smoothedCameraY.value
  cameraOffset.value.x = payloadData.value.world_min.x // Always show full width
}

// Update container height based on window size
const updateContainerHeight = () => {
  containerHeight.value = window.innerHeight - 100 // Leave some margin for UI
}

onMounted(() => {
  updateContainerHeight()
  window.addEventListener('resize', updateContainerHeight)
})

onUnmounted(() => {
  window.removeEventListener('resize', updateContainerHeight)
})

const update = (data: MiniGamePayloadType) => {
  switch (data.gamestatetype()) {
    case GameStateType.MarbleManiaHost: {
      viewState.value = ViewState.MiniGame
      payloadData.value = parseMarbleManiaHostPayload(data)
      // Update camera when simulation is running or finished
      if (payloadData.value.game_phase === 1 || payloadData.value.game_phase === 2) {
        // Initialize smoothed camera position if it hasn't been set yet
        if (smoothedCameraY.value === 0) {
          smoothedCameraY.value = cameraOffset.value.y
        }
        updateCamera()
      } else {
        // Reset camera during placement phase to show the top area
        const topPosition = payloadData.value.world_min.y
        cameraOffset.value = { x: payloadData.value.world_min.x, y: topPosition }
        smoothedCameraY.value = topPosition // Keep smoothed position in sync
      }
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

// ---------- Rendering helpers (center-based with dynamic camera) ----------

const PAD = 10 // border padding

const worldToScreen = (w: MMVec2) => {
  const worldWidth = payloadData.value.world_max.x - payloadData.value.world_min.x
  
  // Calculate how much world space our camera viewport covers
  const cameraWorldHeight = (canvasHeight.value / canvasWidth.value) * worldWidth
  
  // Camera world bounds
  const cameraWorldMin = {
    x: cameraOffset.value.x,
    y: cameraOffset.value.y
  }
  
  // Map world coordinates to screen coordinates within the camera viewport
  const x = (w.x - cameraWorldMin.x) * (canvasWidth.value - 2 * PAD) / worldWidth + PAD
  const y = (w.y - cameraWorldMin.y) * (canvasHeight.value - 2 * PAD) / cameraWorldHeight + PAD
  
  return { x, y }
}

const getEntityCenter = (e: MarbleManiaEntity) => worldToScreen(e.pos)

const rotateLocal = (v: MMVec2, rot: number): MMVec2 => {
  const c = Math.cos(rot), s = Math.sin(rot)
  return { x: v.x * c - v.y * s, y: v.x * s + v.y * c }
}

const add = (a: MMVec2, b: MMVec2): MMVec2 => ({ x: a.x + b.x, y: a.y + b.y })

// Camera culling - check if an entity should be rendered based on camera viewport
const isEntityInCameraView = (entity: MarbleManiaEntity): boolean => {
  const worldWidth = payloadData.value.world_max.x - payloadData.value.world_min.x
  const cameraWorldHeight = (canvasHeight.value / canvasWidth.value) * worldWidth
  
  // Current camera bounds in world coordinates
  const cameraMinX = cameraOffset.value.x
  const cameraMaxX = cameraOffset.value.x + worldWidth
  const cameraMinY = cameraOffset.value.y
  const cameraMaxY = cameraOffset.value.y + cameraWorldHeight
  
  // Calculate entity bounds with some margin for edge cases
  let entityMinX = entity.pos.x
  let entityMaxX = entity.pos.x
  let entityMinY = entity.pos.y
  let entityMaxY = entity.pos.y
  
  // Add entity size-based margins
  if (entity.shape.kind === 'circle') {
    const radius = entity.shape.radius
    entityMinX -= radius
    entityMaxX += radius
    entityMinY -= radius
    entityMaxY += radius
  } else if (entity.shape.kind === 'rect') {
    const hw = entity.shape.width / 2
    const hh = entity.shape.height / 2
    // For rotated rectangles, use conservative bounds
    const maxExtent = Math.max(hw, hh) * 1.5 // Extra margin for rotation
    entityMinX -= maxExtent
    entityMaxX += maxExtent
    entityMinY -= maxExtent
    entityMaxY += maxExtent
  } else if (entity.shape.kind === 'poly') {
    // For polygons, find the maximum extent from vertices
    let maxExtent = 50 // Default margin
    for (const vertex of entity.shape.vertices) {
      const extent = Math.sqrt(vertex.x * vertex.x + vertex.y * vertex.y)
      maxExtent = Math.max(maxExtent, extent)
    }
    entityMinX -= maxExtent
    entityMaxX += maxExtent
    entityMinY -= maxExtent
    entityMaxY += maxExtent
  }
  
  // Add extra margin to prevent pop-in/pop-out
  const margin = 50 // World units
  entityMinX -= margin
  entityMaxX += margin
  entityMinY -= margin
  entityMaxY += margin
  
  // Check if entity bounds intersect with camera bounds
  return !(entityMaxX < cameraMinX || entityMinX > cameraMaxX || 
           entityMaxY < cameraMinY || entityMinY > cameraMaxY)
}

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

// Off-screen marble indicators
const renderOffScreenIndicators = (g: Graphics) => {
  const marbles = getMarbles()
  const cameraMinY = cameraOffset.value.y
  
  // Find marbles that are above the camera view (have lower Y values than camera min)
  const offScreenMarbles = marbles.filter(marble => {
    // Marble is above camera view if its Y position is less than camera minimum Y
    return marble.pos.y < cameraMinY
  })
  
  // Render indicators for off-screen marbles
  offScreenMarbles.forEach(marble => {
    // Convert marble's world X position to screen X
    const screenPos = worldToScreen({ x: marble.pos.x, y: cameraMinY })
    
    // Calculate distance above camera view
    const distanceAbove = Math.round(cameraMinY - marble.pos.y)
    
    // Draw dot at top of screen (use same color as regular marbles)
    const dotY = 20 // 20 pixels from top
    g.lineStyle(2, 0x2d5016) // Same border color as marbles
    g.beginFill(0x4ade80) // Same fill color as marbles
    g.drawCircle(screenPos.x, dotY, 6) // 6 pixel radius dot
    g.endFill()
    
    // Create and render text using PIXI Text
    const textY = dotY + 18
    const distanceText = `${distanceAbove}`
    
    // Draw text background (simplified approach)
    const textWidth = distanceText.length * 8 // Approximate width
    const textHeight = 14
    
    g.lineStyle(0)
    g.beginFill(0x000000, 0.8)
    g.drawRect(screenPos.x - textWidth/2 - 2, textY - textHeight/2 - 1, textWidth + 4, textHeight + 2)
    g.endFill()
    
    // Text will be rendered via HTML overlay (see template)
  })
}

// ---------- Render loops ----------

const render = (g: Graphics) => {
  g.clear()

  // World boundaries - only left and right borders (fixed to world, not camera)
  g.lineStyle(4, 0xffffff)
  
  // Left world boundary
  const leftBoundary = worldToScreen({ x: payloadData.value.world_min.x, y: 0 })
  g.moveTo(leftBoundary.x, 0)
  g.lineTo(leftBoundary.x, canvasHeight.value)
  
  // Right world boundary  
  const rightBoundary = worldToScreen({ x: payloadData.value.world_max.x, y: 0 })
  g.moveTo(rightBoundary.x, 0)
  g.lineTo(rightBoundary.x, canvasHeight.value)

  // drop zone (top band) - only show during placement phase
  if (payloadData.value.game_phase === 0) {
    // Position drop zone to match backend bounds exactly
    const dropZoneLeft = worldToScreen({ x: payloadData.value.world_min.x + 10, y: payloadData.value.world_min.y + 20 })
    const dropZoneRight = worldToScreen({ x: payloadData.value.world_max.x - 10, y: payloadData.value.world_min.y + 120 })
    
    g.lineStyle(2, 0x00ff00)
    g.beginFill(0x00ff00, 0.1)
    g.drawRect(dropZoneLeft.x, dropZoneLeft.y, dropZoneRight.x - dropZoneLeft.x, dropZoneRight.y - dropZoneLeft.y)
    g.endFill()
  }

  // finish line
  const fy = worldToScreen({ x: 0, y: payloadData.value.finish_line_y }).y
  const finishLeft = worldToScreen({ x: payloadData.value.world_min.x, y: payloadData.value.finish_line_y })
  const finishRight = worldToScreen({ x: payloadData.value.world_max.x, y: payloadData.value.finish_line_y })
  
  g.lineStyle(4, 0xff0000)
  g.moveTo(finishLeft.x, fy)
  g.lineTo(finishRight.x, fy)
  g.beginFill(0xff0000)
  for (let x = finishLeft.x; x < finishRight.x; x += 20) {
    if (Math.floor((x - finishLeft.x) / 20) % 2 === 0) {
      g.drawRect(x, fy - 5, 20, 10)
    }
  }
  g.endFill()

  // placement timer
  if (payloadData.value.game_phase === 0 && payloadData.value.placement_time_left > 0) {
    const timerW = 200, timerH = 20, timerX = canvasWidth.value / 2 - timerW / 2, timerY = PAD + 120
    g.lineStyle(2, 0xffffff)
    g.beginFill(0x333333)
    g.drawRect(timerX, timerY, timerW, timerH)
    g.endFill()
    const progress = Math.max(0, payloadData.value.placement_time_left / 15)
    g.beginFill(progress > 0.3 ? 0x00ff00 : 0xff0000)
    g.drawRect(timerX + 2, timerY + 2, (timerW - 4) * progress, timerH - 4)
    g.endFill()
  }

  // obstacles - only render those visible in camera view
  for (const e of getObstacles()) {
    if (!isEntityInCameraView(e)) continue // Skip off-screen obstacles
    
    if (e.shape.kind === 'circle') renderCircleObstacle(g, e)
    else if (e.shape.kind === 'rect') renderRectObstacle(g, e)
    else renderPolyObstacle(g, e)
  }

  // marbles - only render those visible in camera view
  for (const e of getMarbles()) {
    if (!isEntityInCameraView(e)) continue // Skip off-screen marbles
    renderMarble(g, e)
  }

  // off-screen marble indicators - show dots for marbles above the camera view
  renderOffScreenIndicators(g)
}

const renderResults = (g: Graphics) => {
  g.clear()
  g.lineStyle(2, 0x333333)
  g.beginFill(0x1a1a1a, 0.8)
  g.drawRect(0, 0, canvasWidth.value, canvasHeight.value)
  g.endFill()

  // Draw marbles for each result entry
  results.value.results.forEach((result, idx) => {
    const marbleX = canvasWidth.value / 2 - 180 // Position marble to the left of text
    const marbleY = 50 + idx * 60
    const marbleRadius = 16

    // Draw marble (green with highlight like in game)
    g.lineStyle(2, 0x2d5016)
    g.beginFill(0x4ade80)
    g.drawCircle(marbleX, marbleY, marbleRadius)
    g.endFill()

    // Add marble highlight
    g.lineStyle(0)
    g.beginFill(0x86efac, 0.6)
    g.drawCircle(marbleX - marbleRadius * 0.3, marbleY - marbleRadius * 0.3, marbleRadius * 0.3)
    g.endFill()
  })
}

// utils
const getMarbles = () => payloadData.value.entities.filter(e => e.type === 'marble')
const getObstacles = () => payloadData.value.entities.filter(e => e.type === 'obstacle')

// Get off-screen marble indicators data
const getOffScreenMarbles = () => {
  const marbles = getMarbles()
  const cameraMinY = cameraOffset.value.y
  
  return marbles
    .filter(marble => marble.pos.y < cameraMinY)
    .map(marble => {
      const screenPos = worldToScreen({ x: marble.pos.x, y: cameraMinY })
      const distanceAbove = Math.round((cameraMinY - marble.pos.y) / 5) // scale down for readability
      return {
        id: marble.id,
        screenX: screenPos.x,
        distance: distanceAbove,
        playerName: marble.player_name
      }
    })
}

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
  <div ref="containerRef" class="h-full">
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
          <div class="flex flex-col justify-center items-center mt-72">
            <div v-if="payloadData.game_phase === 0" class="text-3xl flex flex-col justify-center items-center">
              <div>Drop your marbles in the green zone!</div>
            </div>
          </div>
        </div>

        <div class="relative">
          <div class="absolute top-0 left-0 w-full h-full bg-black"></div>
          <div class="relative">
            <Application 
              :key="applicationId" 
              :width="canvasWidth" 
              :height="canvasHeight" 
              background-color="black"
            >
              <Graphics :x="0" :y="0" @render="render" />
              
              <!-- Off-screen marble indicator texts -->
              <template v-for="marble in getOffScreenMarbles()" :key="`off-screen-${marble.id}`">
                <Text
                  :position="{ x: marble.screenX, y: 38 }"
                  :text="marble.distance.toString()"
                  :style="{ 
                    fontFamily: ['Arial','sans-serif'], 
                    fontSize: 20, 
                    fill: 'white', 
                    stroke: 'black', 
                    strokeThickness: 3,
                    align: 'center'
                  }"
                  anchor="0.5"
                />
              </template>
              
              <!-- marble labels & finished icons - only for visible marbles -->
              <template v-for="marble in getMarbles().filter(m => isEntityInCameraView(m))" :key="marble.id">
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
          <Application :width="canvasWidth" :height="canvasHeight" background-color="black">
            <Graphics :x="0" :y="0" @render="renderResults" />
            <template v-for="(result, idx) in results.results" :key="result.name">
              <Text
                :position="{ x: getCenteredTextPosition(`${formatOrdinals(result.placement)}. ${result.name}`, canvasWidth / 2 - 100), y: 25 + idx * 60 }"
                :text="`${formatOrdinals(result.placement)}. ${result.name}`"
                :style="{ fontFamily: ['Helvetica','Arial','sans-serif'], fontSize: 24, fill: result.has_finished ? 'white' : 'gray', stroke: 'black', strokeThickness: 3 }"
              />
              <Text
                v-if="result.has_finished"
                :position="{ x: getCenteredTextPosition(`Time: ${result.time_to_finish.toFixed(2)}s`, canvasWidth / 2 - 100), y: 50 + idx * 60 }"
                :text="`Time: ${result.time_to_finish.toFixed(2)}s`"
                :style="{ fontFamily: ['Helvetica','Arial','sans-serif'], fontSize: 16, fill: 'lightblue', stroke: 'black', strokeThickness: 2 }"
              />
              <Text
                v-else
                :position="{ x: getCenteredTextPosition('Did not finish', canvasWidth / 2 - 100), y: 50 + idx * 60 }"
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
