<script lang="ts" setup>
import { ref, defineProps } from 'vue'
import { type IntroductionData } from '@/components/introduction/Introduction.vue'
import Introduction from '@/components/introduction/Introduction.vue'
import {
  GameStateType,
  MiniGameIntroductionPayload,
  type MiniGamePayloadType
} from '@/flatbuffers/messageClass'
import { type MarbleManiaData, type MarbleManiaResult } from './MarbleManiaModels'
import { parseMarbleManiaHostPayload, parseMarbleManiaResultPayload } from './MarbleManiaProcessor'
import { Application } from 'vue3-pixi'
import { Graphics, Sprite, Text, TextStyle, CanvasTextMetrics } from 'pixi.js'
import { watch } from 'vue'

// Simple sprite mapping (for marbles only)
const getMarbleSprite = () => '/assets/games/crazyCounting/partyhat.svg'
const getMarbleSpriteDimensions = () => ({ width: 30, height: 30 })

// Render obstacles as Graphics shapes
const renderCircleObstacle = (graphics: Graphics, obstacle: any) => {
  const pos = getEntityScreenPosition(obstacle)
  const radius = (obstacle.width || 40) / 2

  graphics.lineStyle(2, 0x333333)
  graphics.beginFill(0x8b4513) // Brown color for obstacles
  graphics.drawCircle(pos.x + radius, pos.y + radius, radius)
  graphics.endFill()
}

const renderRectangleObstacle = (graphics: Graphics, obstacle: any) => {
  const pos = getEntityScreenPosition(obstacle)
  const width = obstacle.width || 40
  const height = obstacle.height || 40
  const rotation = obstacle.rotation || 0

  // For rotated rectangles, we need to calculate the corners
  const centerX = pos.x + width / 2
  const centerY = pos.y + height / 2
  
  if (rotation === 0) {
    // Simple case - no rotation
    graphics.lineStyle(2, 0x333333)
    graphics.beginFill(0x8b4513) // Brown color for obstacles
    graphics.drawRect(pos.x, pos.y, width, height)
    graphics.endFill()
  } else {
    // Rotated rectangle - draw as polygon
    const halfWidth = width / 2
    const halfHeight = height / 2
    const cos = Math.cos(rotation)
    const sin = Math.sin(rotation)
    
    const corners = [
      { x: -halfWidth, y: -halfHeight },
      { x: halfWidth, y: -halfHeight },
      { x: halfWidth, y: halfHeight },
      { x: -halfWidth, y: halfHeight }
    ]
    
    const rotatedCorners = corners.map(corner => ({
      x: centerX + corner.x * cos - corner.y * sin,
      y: centerY + corner.x * sin + corner.y * cos
    }))
    
    graphics.lineStyle(2, 0x333333)
    graphics.beginFill(0x8b4513) // Brown color for obstacles
    graphics.drawPolygon(rotatedCorners.flatMap(c => [c.x, c.y]))
    graphics.endFill()
  }
}

defineProps<{
  width: number
  height: number
}>()

enum ViewState {
  None,
  Introduction,
  MiniGame,
  Results
}

const viewState = ref<ViewState>(ViewState.None)

// introduction
const intro = ref<IntroductionData>({
  title: '',
  description: '',
  time_left: 0
})

// host payload data
const payloadData = ref<MarbleManiaData>({
  entities: [],
  finish_line_y: 0,
  game_phase: 0,
  placement_time_left: 0,
})

// final results
const results = ref<MarbleManiaResult>({
  results: []
})

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
      break;
    }
  }
  return []
}

// render the game world
const render = (graphics: Graphics) => {
  graphics.clear()
  
  const canvasWidth = 800
  const canvasHeight = 600
  
  // Draw world boundaries
  graphics.lineStyle(4, 0xffffff)
  graphics.drawRect(10, 10, canvasWidth - 20, canvasHeight - 20)
  
  // Draw drop zone (top area)
  if (payloadData.value.game_phase === 0) { // Placement phase
    graphics.lineStyle(2, 0x00ff00)
    graphics.beginFill(0x00ff00, 0.1)
    graphics.drawRect(60, 20, canvasWidth - 120, 100)
    graphics.endFill()
    
    // Add drop zone text
    graphics.lineStyle(0)
    graphics.beginFill(0x00ff00)
    graphics.drawRect(canvasWidth / 2 - 50, 30, 100, 20)
    graphics.endFill()
  }
  
  // Draw finish line
  const finishY = transformWorldToScreen(0, payloadData.value.finish_line_y).y
  graphics.lineStyle(4, 0xff0000)
  graphics.moveTo(10, finishY)
  graphics.lineTo(canvasWidth - 10, finishY)
  
  // Draw finish line pattern (checkered)
  graphics.beginFill(0xff0000)
  for (let x = 10; x < canvasWidth - 10; x += 20) {
    if (Math.floor((x - 10) / 20) % 2 === 0) {
      graphics.drawRect(x, finishY - 5, 20, 10)
    }
  }
  graphics.endFill()
  
  // Draw placement timer if in placement phase
  if (payloadData.value.game_phase === 0 && payloadData.value.placement_time_left > 0) {
    const timerWidth = 200
    const timerHeight = 20
    const timerX = canvasWidth / 2 - timerWidth / 2
    const timerY = 130
    
    // Timer background
    graphics.lineStyle(2, 0xffffff)
    graphics.beginFill(0x333333)
    graphics.drawRect(timerX, timerY, timerWidth, timerHeight)
    graphics.endFill()
    
    // Timer fill
    const progress = Math.max(0, payloadData.value.placement_time_left / 15) // Assuming 15 seconds max
    graphics.beginFill(progress > 0.3 ? 0x00ff00 : 0xff0000)
    graphics.drawRect(timerX + 2, timerY + 2, (timerWidth - 4) * progress, timerHeight - 4)
    graphics.endFill()
  }
  
  // Draw obstacles
  for (const obstacle of getObstacles()) {
    if (obstacle.is_circle) {
      renderCircleObstacle(graphics, obstacle)
    } else {
      renderRectangleObstacle(graphics, obstacle)
    }
  }
}

// Render results background
const renderResults = (graphics: Graphics) => {
  graphics.clear()
  
  const canvasWidth = 800
  const canvasHeight = 600
  
  // Draw background with subtle pattern
  graphics.lineStyle(2, 0x333333)
  graphics.beginFill(0x1a1a1a, 0.8)
  graphics.drawRect(0, 0, canvasWidth, canvasHeight)
  graphics.endFill()
  
  // Draw title background
  graphics.lineStyle(3, 0xffd700)
  graphics.beginFill(0x333333, 0.9)
  graphics.drawRoundedRect(canvasWidth / 2 - 200, 50, 400, 60, 10)
  graphics.endFill()
}

// Transform world coordinates to screen coordinates
const transformWorldToScreen = (worldX: number, worldY: number) => {
  const canvasWidth = 800
  const canvasHeight = 600
  const worldWidth = 800  // Assuming world coordinates from -400 to 400
  const worldHeight = 700 // Assuming world coordinates from -300 to 400
  
  const screenX = (worldX + 400) * (canvasWidth - 20) / worldWidth + 10
  const screenY = (worldY + 300) * (canvasHeight - 20) / worldHeight + 10
  
  return { x: screenX, y: screenY }
}

// Get entity screen position
const getEntityScreenPosition = (entity: any) => {
  return transformWorldToScreen(entity.x_pos, entity.y_pos)
}

// Separate marbles and obstacles from entities
const getMarbles = () => {
  return payloadData.value.entities.filter(entity => entity.entity_type === 0)
}

const getObstacles = () => {
  return payloadData.value.entities.filter(entity => entity.entity_type === 1)
}

const applicationId = ref(0)

watch(
  () => payloadData.value.entities,
  (newVal, oldVal) => {
    // if an entity is added/removed, force rerender of the application
    if (!oldVal || !newVal) return
    if (newVal.length !== oldVal.length) {
      applicationId.value += 1;
    }
  },
  { immediate: true }
)

// Text style for labels
const textStyle = new TextStyle({
  fontFamily: ['Helvetica', 'Arial', 'sans-serif'],
  fontSize: 14,
  fill: 'white',
  stroke: 'black',
  // @ts-expect-error: PixiJS typing issue
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

const formatOrdinals = (n: number) => {
  const rule = pr.select(n)
  const suffix = suffixes.get(rule)
  return `${n}${suffix}`
}

defineExpose({
  update
})

</script>
<template>
  <div class="h-full">
    <div v-if="viewState == ViewState.Introduction">
      <Introduction :data="intro" logoSVG="/assets/games/MarbleMania/MarbleManiaLogo.svg" />
    </div>
    <div v-else-if="viewState == ViewState.MiniGame">
      <div class="flex flex-col h-full w-full justify-center items-center">
        <div class="text-4xl m-6">
          <span v-if="payloadData.game_phase === 0">Place Your Marbles! Time: {{ Math.ceil(payloadData.placement_time_left) }}s</span>
          <span v-else-if="payloadData.game_phase === 1">Marbles are Rolling!</span>
          <span v-else>Simulation Complete</span>
        </div>
        <div class="absolute top-0 text text-white z-10">
          <div class="flex flex-col justify-center items-center mt-32">
            <div v-if="payloadData.game_phase === 0" class="text-3xl flex flex-col justify-center items-center">
              <div>
                Drop your marbles in the green zone!
              </div>
            </div>
          </div>
        </div>
        <div class="relative">
          <!-- black background, for when the application is rerendering -->
          <div class="absolute top-0 left-0 w-full h-full bg-black"></div>
          <div class="relative">
            <Application :key="applicationId" :width="800" :height="600" background-color="black">
              <Graphics :x="0" :y="0" @render="render" />
              <!-- Render marbles -->
              <template v-for="marble in getMarbles()" :key="marble.id">
                <Sprite
                  :position="getEntityScreenPosition(marble)"
                  :width="getMarbleSpriteDimensions().width"
                  :height="getMarbleSpriteDimensions().height"
                  :texture="getMarbleSprite()"
                />
                <!-- Marble ID text -->
                <Text
                  :position="{
                    x: getCenteredTextPosition(marble.player_name || marble.id, getEntityScreenPosition(marble).x + getMarbleSpriteDimensions().width / 2),
                    y: getEntityScreenPosition(marble).y - 20
                  }"
                  :text="marble.player_name || marble.id"
                  :style="{fontFamily: ['Helvetica', 'Arial', 'sans-serif'], fontSize: 18, fill: 'white', stroke: 'black', strokeThickness: 4}"
                />
                <!-- Finished indicator -->
                <Sprite
                  v-if="marble.is_finished"
                  :position="{
                    x: getEntityScreenPosition(marble).x,
                    y: getEntityScreenPosition(marble).y
                  }"
                  :width="40"
                  :height="40"
                  texture="/assets/games/marbleMania/checkmark.png"
                />
              </template>
              <!-- Obstacles are now rendered via Graphics in the render function -->
            </Application>
          </div>
        </div>
      </div>
    </div>
    <div v-else-if="viewState == ViewState.Results" class="flex flex-col h-full w-full justify-center items-center">
      <div class="text-4xl m-6">
        Marble Mania Results
      </div>
      <div class="relative">
        <div class="absolute top-0 left-0 w-full h-full bg-black"></div>
        <div class="relative">
          <Application :width="800" :height="600" background-color="black">
            <Graphics :x="0" :y="0" @render="renderResults" />
            <!-- Results display -->
            <template v-for="(result, index) in results.results" :key="result.name">
              <Text
                :position="{
                  x: getCenteredTextPosition(`${formatOrdinals(result.placement)}. ${result.name}`, 400),
                  y: 150 + index * 60
                }"
                :text="`${formatOrdinals(result.placement)}. ${result.name}`"
                :style="{
                  fontFamily: ['Helvetica', 'Arial', 'sans-serif'],
                  fontSize: 24,
                  fill: result.has_finished ? 'white' : 'gray',
                  stroke: 'black',
                  strokeThickness: 3
                }"
              />
              <Text
                v-if="result.has_finished"
                :position="{
                  x: getCenteredTextPosition(`Time: ${result.time_to_finish.toFixed(2)}s`, 400),
                  y: 175 + index * 60
                }"
                :text="`Time: ${result.time_to_finish.toFixed(2)}s`"
                :style="{
                  fontFamily: ['Helvetica', 'Arial', 'sans-serif'],
                  fontSize: 16,
                  fill: 'lightblue',
                  stroke: 'black',
                  strokeThickness: 2
                }"
              />
              <Text
                v-else
                :position="{
                  x: getCenteredTextPosition('Did not finish', 400),
                  y: 175 + index * 60
                }"
                text="Did not finish"
                :style="{
                  fontFamily: ['Helvetica', 'Arial', 'sans-serif'],
                  fontSize: 16,
                  fill: 'red',
                  stroke: 'black',
                  strokeThickness: 2
                }"
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