<script setup lang="ts">
import { ref, defineProps, toRefs, computed } from 'vue'
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
const time = ref(0)

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

      if (newValue > value.value) value.value = newValue
      if (bailedPlayers.value.length != bailed_players.length) {
        bailedPlayers.value = bailed_players
      }

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

const yHeight = computed(() => height.value - yMarginBottom)

function interpPosition(position: IPointData): [number, number] {
  const xScale = width.value / time.value
  const yScale = yHeight.value / value.value
  const newX = position.x * xScale
  const newY = yHeight.value - position.y * yScale

  return [newX, newY]
}

function render(graphics: Graphics) {
  graphics.clear()

  graphics.lineStyle(10, colorStore.colorPalette.primary.base.number)
  graphics.moveTo(0, yHeight.value)
  points.value.forEach((point) => {
    graphics.lineTo(...interpPosition(point))
  })

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
