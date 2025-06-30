<script setup lang="ts">
import { ref, defineProps, toRefs, computed } from 'vue'
import MoneyCounter from './components/MoneyCounter.vue'
import ResultGraph from './components/ResultGraph.vue'
import Introduction from '../introduction/Introduction.vue'
import { Graphics, type PointData } from 'pixi.js'
import { Application } from 'vue3-pixi'
import type { MiniGamePayloadType } from '@/flatbuffers/mini-game-payload-type'
import { GameStateType } from '@/flatbuffers/game-state-type'
import { MiniGameIntroductionPayload } from '@/flatbuffers/mini-game-introduction-payload'
import { useColorStore } from '@/stores/colorStore'
import {
  parseBusinessBailoutHostPayload,
  parseBusinessBailoutResultPayload,
  type BailedPlayer
} from './parser'

enum ViewState {
  None,
  Introduction,
  MiniGame,
  Results
}

type IntroductionData = {
  title: string
  description: string
  time_left: number
}
const intro = ref<IntroductionData>({
  title: '',
  description: '',
  time_left: 0
})

const colorStore = useColorStore()

const viewState = ref<ViewState>(ViewState.None)
const value = computed(() => points.value[points.value.length - 1].y)
const maxValue = computed(() => Math.max(...points.value.map((point) => point.y)))
const time = computed(() => points.value[points.value.length - 1].x)
// Angle between value and last value
const angle = computed(() => {
  const numPoints = points.value.length
  if (numPoints < 2) return 0

  // Choose how many points to look back for a smoother angle
  const lookbackSteps = Math.min(5, numPoints - 1)

  const currentPoint = interpPosition(points.value[numPoints - 1])
  const earlierPoint = interpPosition(points.value[numPoints - 1 - lookbackSteps])

  // Calculate angle from earlier point to current point
  return Math.atan2(currentPoint[0] - earlierPoint[0], currentPoint[1] - earlierPoint[1])
})
const size = ref(75)

const props = defineProps<{
  height: number
  width: number
}>()
const { height, width } = toRefs(props)

const points = ref<PointData[]>([])
const bailedPlayers = ref<BailedPlayer[]>([])

const createPointData = (value: number, time: number): PointData => ({ x: time, y: value })

function update(payload: MiniGamePayloadType) {
  switch (payload.gamestatetype()) {
    case GameStateType.MiniGameIntroduction: {
      viewState.value = ViewState.Introduction
      const introPayload: MiniGameIntroductionPayload = payload.gamestatepayload(
        new MiniGameIntroductionPayload()
      )
      intro.value = {
        title: introPayload.name() || '',
        description: introPayload.instruction() || '',
        time_left: Number(introPayload.timeLeft())
      }
      break
    }
    case GameStateType.BusinessBailoutHost: {
      if (viewState.value === ViewState.Results) break
      viewState.value = ViewState.MiniGame
      const {
        value: newValue,
        bailed_players,
        time: newTime
      } = parseBusinessBailoutHostPayload(payload)

      if (bailedPlayers.value.length != bailed_players.length) {
        bailedPlayers.value = bailed_players
      }

      points.value.push(createPointData(newValue, newTime))
      break
    }
    case GameStateType.BusinessBailoutResult: {
      viewState.value = ViewState.Results

      const { submittedPlayers } = parseBusinessBailoutResultPayload(payload)

      bailedPlayers.value = submittedPlayers
      break
    }
    default:
      throw new Error(`Unknown gamestatetype: ${payload.gamestatetype()}`)
  }
}

defineExpose({ update })

const yMargin = 25
const xMargin = 25

const xWidth = computed(() => width.value - xMargin * 2)
const yHeight = computed(() => height.value - yMargin * 2)

function interpPosition(position: PointData): [number, number] {
  const xStep = xWidth.value / time.value
  const yStep = yHeight.value / maxValue.value
  const newX = position.x * xStep + xMargin
  const newY = height.value - (position.y * yStep + yMargin)

  return [newX, newY]
}

function renderMinigame(graphics: Graphics) {
  graphics.clear()

  graphics.lineStyle(10, colorStore.colorPalette.primary.base.number)
  graphics.moveTo(...interpPosition(points.value[0]))
  points.value.forEach((point) => {
    graphics.lineTo(...interpPosition(point))
  })

  // Draw the x axis
  graphics.lineStyle(4, 0xffffff)
  graphics.moveTo(...interpPosition({ x: 0, y: 0 }))
  for (let timeIncrement = 0; timeIncrement < time.value; timeIncrement += 2000) {
    const point = { x: timeIncrement, y: 0 }
    const position = interpPosition(point)
    graphics.lineTo(...position)
    graphics.lineTo(position[0], position[1] + 10)
    graphics.lineTo(position[0], position[1] - 10)
    graphics.lineTo(...position)
  }
  graphics.lineTo(...interpPosition({ x: time.value, y: 0 }))

  graphics.lineStyle(4, 0x100000)
  // Draw the y axis
  graphics.lineStyle(4, 0xffffff)
  graphics.moveTo(xWidth.value + xMargin, interpPosition({ x: 0, y: yMargin })[1])
  for (
    let valueIncrement = 0;
    interpPosition({ x: 0, y: valueIncrement })[1] > 10;
    valueIncrement += 4000
  ) {
    const point = { x: 0, y: valueIncrement }
    graphics.lineTo(xWidth.value + xMargin, interpPosition(point)[1])
    graphics.lineTo(xWidth.value + 10 + xMargin, interpPosition(point)[1])
    graphics.lineTo(xWidth.value - 10 + xMargin, interpPosition(point)[1])
    graphics.lineTo(xWidth.value + xMargin, interpPosition(point)[1])
  }
  graphics.lineTo(xWidth.value + xMargin, yMargin)

  graphics.lineStyle(4, 0xff0000)
  bailedPlayers.value.forEach((bailedPlayer) => {
    // Draw a vertical line at the time the player bailed
    const [x, y] = interpPosition({ x: bailedPlayer.time, y: bailedPlayer.value })
    graphics.moveTo(x, interpPosition({ x: 0, y: 0 })[1])
    graphics.lineTo(x, y)

    // Draw a circle at the point where the player bailed
    graphics.beginFill(0xff0000)
    graphics.drawCircle(x, y, 15)
    graphics.endFill()
  })
}
</script>
<template>
  <template v-if="viewState === ViewState.Introduction">
    <div>
      <Introduction :data="intro" logoSVG="/assets/games/businessBailout/businessBailoutLogo.svg" />
    </div>
  </template>
  <template v-else-if="viewState === ViewState.MiniGame">
    <div class="absolute h-full w-full">
      <Application :height :width background-color="black">
        <Graphics :x="0" :y="0" @render="renderMinigame" />
        <Text
          v-for="bailedPlayer in bailedPlayers"
          :key="bailedPlayer.name"
          :style="{ fill: 'white' }"
          :anchor-x="1.1"
          :anchor-y="0.07"
          :x="interpPosition({ x: bailedPlayer.time, y: bailedPlayer.value })[0] + 10"
          :y="interpPosition({ x: bailedPlayer.time, y: bailedPlayer.value })[1]"
          :text="bailedPlayer.name"
          :rotation="Math.PI * 0.25"
        />
        <Sprite
          :position-x="interpPosition(points[points.length - 1])[0]"
          :position-y="interpPosition(points[points.length - 1])[1]"
          :width="size"
          :anchor-x="0.5"
          :height="size * 2"
          :rotation="Math.PI - angle"
          texture="/assets/games/businessBailout/rocket.svg"
        />
      </Application>
    </div>
    <div class="flex ml-4 mt-4 w-full justify-start items-start">
      <div class="z-20 justify-start items-start">
        <span class="text text-[2vw] text-white ">Sell when the price is the highest!<br> But whatch out for the big crash.</span>
      </div>
    </div>
    <div class="flex w-full justify-center items-center mt" style="margin-top: -6rem;">
      <MoneyCounter :value />
    </div>
  </template>
  <template v-if="viewState === ViewState.Results">
    <div class="absolute h-full w-full">
      <ResultGraph :points :height :width :bailed-players="bailedPlayers" />
    </div>
  </template>
  <template v-else> </template>
</template>
