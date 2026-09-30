<script lang="ts" setup>
import { ref, computed } from 'vue'
import { type IntroductionData } from '@/components/introduction/Introduction.vue'
import Introduction from '@/components/introduction/Introduction.vue'
import {
  GameStateType,
  MiniGameIntroductionPayload,
  type MiniGamePayloadType
} from '@/flatbuffers/messageClass'
import {
  type HighwayHustleData,
  type HighwayHustleResult,
  type HighwayHustleSnapshot
} from './HighwayHustleModels'
import {
  parseHighwayHustleHostPayload,
  parseHighwayHustleResultPayload
} from './HighwayHustleProcessor'
import HighwayHustleGameView from './HighwayHustleGameView.vue'

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
const payloadData = ref<HighwayHustleData>({
  players: [],
  obstacles: [],
  distance: 0
})

// final results
const results = ref<HighwayHustleResult>({
  results: []
})

const snapshot = computed<HighwayHustleSnapshot>(() => ({
  phase: viewState.value === ViewState.Results ? 'results' : 'race',
  payload: payloadData.value,
  results: results.value
}))

const update = (data: MiniGamePayloadType) => {
  switch (data.gamestatetype()) {
    case GameStateType.HighwayHustleHost: {
      viewState.value = ViewState.MiniGame

      payloadData.value = parseHighwayHustleHostPayload(data)
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
    case GameStateType.HighwayHustleResult: {
      viewState.value = ViewState.Results
      results.value = parseHighwayHustleResultPayload(data)
      break
    }
  }
  return []
}

defineExpose({
  update,
  ...(import.meta.env.DEV
    ? {
        // Mirrors update()'s real HighwayHustleHost/HighwayHustleResult dispatch (same
        // synchronous viewState/payloadData assignment) — for driving the real prop-passing
        // path from a dev harness without building fake FlatBuffers.
        __devUpdateHost: (data: HighwayHustleData) => {
          viewState.value = ViewState.MiniGame
          payloadData.value = data
        },
        __devUpdateResult: (data: HighwayHustleResult) => {
          viewState.value = ViewState.Results
          results.value = data
        }
      }
    : {})
})
</script>
<template>
  <div class="h-full">
    <div v-if="viewState == ViewState.Introduction">
      <Introduction :data="intro" logoSVG="/assets/games/highwayHustle/highwayHustleLogo.svg" />
    </div>
    <div
      v-else-if="viewState == ViewState.MiniGame || viewState == ViewState.Results"
      class="h-full"
    >
      <div class="flex flex-col h-full w-full items-center">
        <div class="text-4xl m-6 shrink-0">
          <span v-if="viewState == ViewState.MiniGame"
            >Score: {{ Math.max(0, Math.round(payloadData.distance / 10)) }}</span
          >
          <!-- empty span to keep the layout consistent -->
          <span v-else>Minigame Result</span>
        </div>
        <div class="absolute top-0 text text-white z-10">
          <div class="flex flex-col justify-center items-center mt-64">
            <div
              v-if="Math.round(payloadData.distance / 10) < 0 && viewState != ViewState.Results"
              class="text-4xl flex flex-col justify-center items-center"
            >
              <div>Check on your screen which car you are!</div>
              <div>
                Racing starts in {{ Math.abs(Math.round(payloadData.distance / 1000)) }} seconds
              </div>
            </div>
          </div>
        </div>
        <div class="relative w-full flex-1 min-h-0 bg-black">
          <HighwayHustleGameView :snapshot="snapshot" />
        </div>
      </div>
    </div>
  </div>
</template>
