<script setup lang="ts">
import { ref, shallowRef, computed } from 'vue'
import MoneyCounter from './components/MoneyCounter.vue'
import ResultGraph from './components/ResultGraph.vue'
import Introduction from '../introduction/Introduction.vue'
import type { PointData } from 'pixi.js'
import type { MiniGamePayloadType } from '@/flatbuffers/mini-game-payload-type'
import { GameStateType } from '@/flatbuffers/game-state-type'
import { MiniGameIntroductionPayload } from '@/flatbuffers/mini-game-introduction-payload'
import {
  parseBusinessBailoutHostPayload,
  parseBusinessBailoutResultPayload,
  type BailedPlayer,
  type BusinessBailoutHostData
} from './parser'
import BusinessBailoutGameView from './BusinessBailoutGameView.vue'

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

const viewState = ref<ViewState>(ViewState.None)

defineProps<{
  height: number
  width: number
}>()

const points = shallowRef<PointData[]>([])
const bailedPlayers = shallowRef<BailedPlayer[]>([])

const value = computed(() => points.value[points.value.length - 1]?.y ?? 0)

const hostData = computed<BusinessBailoutHostData>(() => ({
  points: points.value,
  bailed_players: bailedPlayers.value
}))

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

      points.value = [...points.value, createPointData(newValue, newTime)]
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

defineExpose({
  update,
  ...(import.meta.env.DEV
    ? {
        __devUpdateHost: (newValue: number, newTime: number, newBailed: BailedPlayer[]) => {
          viewState.value = ViewState.MiniGame
          if (bailedPlayers.value.length != newBailed.length) {
            bailedPlayers.value = newBailed
          }
          points.value = [...points.value, createPointData(newValue, newTime)]
        },
        __devUpdateResults: (submittedPlayers: BailedPlayer[]) => {
          viewState.value = ViewState.Results
          bailedPlayers.value = submittedPlayers
        }
      }
    : {})
})
</script>
<template>
  <template v-if="viewState === ViewState.Introduction">
    <div>
      <Introduction :data="intro" logoSVG="/assets/games/businessBailout/businessBailoutLogo.svg" />
    </div>
  </template>
  <template v-else-if="viewState === ViewState.MiniGame">
    <div class="absolute h-full w-full">
      <BusinessBailoutGameView :host-data="hostData" />
    </div>
    <div class="flex ml-4 mt-4 w-full justify-start items-start">
      <div class="z-20 justify-start items-start">
        <span class="text text-[2vw] text-white"
          >Sell when the price is the highest!<br />
          But whatch out for the big crash.</span
        >
      </div>
    </div>
    <div class="flex w-full justify-center items-center mt" style="margin-top: -6rem">
      <MoneyCounter :value />
    </div>
  </template>
  <template v-if="viewState === ViewState.Results">
    <div class="absolute h-full w-full">
      <ResultGraph :points :bailed-players="bailedPlayers" />
    </div>
  </template>
  <template v-else> </template>
</template>
