<script setup lang="ts">
import { ref, defineProps, toRefs, computed, watchEffect } from 'vue'
import TimeComponent from '../TimeComponent.vue'
import Introduction from '../introduction/Introduction.vue'
import { Graphics, type IPointData } from 'pixi.js'
import { Application } from 'vue3-pixi'
import type { MiniGamePayloadType } from '@/flatbuffers/mini-game-payload-type'
import { GameStateType } from '@/flatbuffers/game-state-type'
import { MiniGameIntroductionPayload } from '@/flatbuffers/mini-game-introduction-payload'
import { useColorStore } from '@/stores/colorStore'
import { parseBusinessBailoutHostPayload, type BailedPlayer } from './parser'

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
const value = ref(0)
const maxValue = ref(0)
const time = ref(0)
const angle = ref(0)

const props = defineProps<{
  height: number
  width: number
}>()
const { height, width } = toRefs(props)

const points = ref<IPointData[]>([])
const bailedPlayers = ref<BailedPlayer[]>([])

const createPointData = (value: number, time: number): IPointData => ({ x: time, y: value })

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
      viewState.value = ViewState.MiniGame
      const {
        value: newValue,
        bailed_players,
        time: newTime
      } = parseBusinessBailoutHostPayload(payload)

      if (bailedPlayers.value.length != bailed_players.length)
        console.log('bailed_players', bailed_players)

      if (bailedPlayers.value.length != bailed_players.length) {
        bailedPlayers.value = bailed_players
      }

      // Calculate the angle between the last point and the new point
      angle.value = Math.atan2(newValue - value.value, newTime - time.value)
      if (newValue > maxValue.value) maxValue.value = newValue
      value.value = newValue
      time.value = newTime

      points.value.push(createPointData(newValue, newTime))
      break
    }
    default:
      throw new Error(`Unknown gamestatetype: ${payload.gamestatetype()}`)
  }
}

defineExpose({ update })

const yMarginBottom = 100
const xMargin = 25

const xWidth = computed(() => width.value - xMargin * 2)
const yHeight = computed(() => height.value - yMarginBottom)
// Scale the position based on the angle between the last point and the new point
const yScale = computed(() => (yHeight.value / (Math.PI / 2)) * angle.value)

function interpPosition(position: IPointData): [number, number] {
  const xStep = xWidth.value / time.value
  const yStep = yScale.value / maxValue.value
  const newX = position.x * xStep + xMargin
  const newY = yHeight.value - position.y * yStep

  return [newX, newY]
}

function render(graphics: Graphics) {
  graphics.clear()

  graphics.lineStyle(10, colorStore.colorPalette.primary.base.number)
  graphics.moveTo(xMargin, yHeight.value)
  points.value.forEach((point) => {
    graphics.lineTo(...interpPosition(point))
  })

  graphics.lineStyle(4, 0x000000)
  graphics.moveTo(xMargin, yHeight.value)
  for (let timeIncrement = 0; timeIncrement < time.value; timeIncrement += 2000) {
    const point = { x: timeIncrement, y: 0 }
    graphics.lineTo(interpPosition(point)[0], yHeight.value)
    graphics.lineTo(interpPosition(point)[0], yHeight.value + 10)
    graphics.lineTo(interpPosition(point)[0], yHeight.value - 10)
    graphics.lineTo(interpPosition(point)[0], yHeight.value)
  }
  graphics.lineTo(width.value - xMargin, yHeight.value)

  graphics.lineStyle(4, 0x100000)
  graphics.lineStyle(4, 0x000000)
  graphics.moveTo(xWidth.value + xMargin, yHeight.value)
  for (let valueIncrement = 0; valueIncrement < value.value; valueIncrement += 4000) {
    const point = { x: 0, y: valueIncrement }
    graphics.lineTo(xWidth.value + xMargin, interpPosition(point)[1])
    graphics.lineTo(xWidth.value + 10 + xMargin, interpPosition(point)[1])
    graphics.lineTo(xWidth.value - 10 + xMargin, interpPosition(point)[1])
    graphics.lineTo(xWidth.value + xMargin, interpPosition(point)[1])
  }
  graphics.lineTo(xWidth.value + xMargin, 10)

  graphics.lineStyle(4, 0xff0000)
  bailedPlayers.value.forEach((bailedPlayer) => {
    // Draw a vertical line at the time the player bailed
    const [x, y] = interpPosition({ x: bailedPlayer.time, y: 0 })
    graphics.moveTo(x, y)
    graphics.lineTo(x, 0)

    // Draw a circle at the point where the player bailed
    graphics.beginFill(0xff0000)
    graphics.drawCircle(x, y, 10)
    graphics.endFill()
  })
}
</script>
<template>
  <template v-if="viewState === ViewState.Introduction">
    <div>
      <Introduction :data="intro" logoSVG="/assets/games/crazyCounting/crazyCountingLogo.svg" />
    </div>
  </template>
  <template v-else-if="viewState === ViewState.MiniGame">
    <div class="absolute h-full w-full">
      <Application :height :width background-color="white">
        <Graphics :x="0" :y="0" @render="render" />
        <Text
          v-for="bailedPlayer in bailedPlayers"
          :key="bailedPlayer.name"
          :x="interpPosition({ x: bailedPlayer.time, y: 0 })[0] + 10"
          :y="interpPosition({ x: bailedPlayer.time, y: 0 })[1]"
          :text="bailedPlayer.name"
          :rotation="Math.PI * 0.25"
        />
      </Application>
    </div>
    <div class="flex m-4">
      <div>
        <TimeComponent :time-left="time" />
      </div>
    </div>
  </template>
  <template v-else>
    <h1>No data</h1>
  </template>
</template>
