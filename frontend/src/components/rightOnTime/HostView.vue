<script lang="ts" setup>
import { ref, defineProps } from 'vue'
import { type IntroductionData } from '@/components/introduction/Introduction.vue'
import Introduction from '@/components/introduction/Introduction.vue'
import {
  GameStateType,
  MiniGameIntroductionPayload,
  type MiniGamePayloadType
} from '@/flatbuffers/messageClass'
import {
  type RightOnTimeData,
  type RightOnTimeRoundResults,
  type RightOnTimeResults,
} from './RightOnTimeModels'
import { NScrollbar, NCard } from 'naive-ui'
import { parseRightOnTimePayload, parseRightOnTimeResults, parseRightOnTimeRoundResults } from './RightOnTimeProcessor'
import FlipClock from './components/FlipClock.vue';

const props = defineProps<{
  width: number
  height: number
}>()

enum ViewState {
  None,
  Introduction,
  MiniGame,
  RoundResults,
  Results
}

const viewState = ref<ViewState>(ViewState.None)

// introduction
const intro = ref<IntroductionData>({
  title: '',
  description: '',
  time_left: 0
})

const payloadData = ref<RightOnTimeData>({
  round: 0,
  target: 0,
  time: 0,
  fade_out: false,
  submitted: []
})

// round results
const roundResultsData = ref<RightOnTimeRoundResults>({
  round: 0,
  target: 0,
  results: []
})

// final results
const resultsData = ref<RightOnTimeResults>({
  results: []
})

const update = (data: MiniGamePayloadType) => {
  switch (data.gamestatetype()) {
    case GameStateType.RightOnTime: {
      viewState.value = ViewState.MiniGame

      payloadData.value = parseRightOnTimePayload(data)
      break
    }
    case GameStateType.RightOnTimeRoundResult: {
      viewState.value = ViewState.RoundResults

      roundResultsData.value = parseRightOnTimeRoundResults(data)
      break
    }
    case GameStateType.RightOnTimeResult: {
      viewState.value = ViewState.Results

      resultsData.value = parseRightOnTimeResults(data)
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
  }
  return []
}

defineExpose({
  update
})

const currentNumber = ref(0)
</script>
<template>
  <div class="h-full">
    <div v-if="viewState == ViewState.Introduction">
      <Introduction :data="intro" logoSVG="/assets/games/rightOnTime/rightOnTimeLogo.svg" />
    </div>
    <div v-else-if="viewState == ViewState.MiniGame">
      <div class="flex h-full w-full justify-center items-center">
        <div class="grid grid-cols-2 h-full w-full m-auto mt-16">
          <div class="flex justify-center">
            <span class="text-6xl flex justify-center mt-7 mr-6">Current round: </span>
            <span class="text-primary text-9xl flex justify-center">{{ payloadData.round }}</span>
          </div>
          <div class="flex justify-center">
            <span class="text-6xl flex justify-center mt-7 mr-6">Target: </span>
            <span class="text-primary text-9xl flex justify-center">{{ (payloadData.target / 1000).toFixed(0) }}s</span>
          </div>
        </div>
      </div>
      <div class="flex h-full w-full justify-center items-center">
        <flip-clock :number="Number((payloadData.time / 1000).toFixed(0))" />
        <span class="font-mono text-primary" style="font-size: 250px;" :class="{ 'fade-out-button': payloadData.fade_out }"> {{ (payloadData.time / 1000).toFixed(0) }}s</span>
        <flip-clock :number="currentNumber" />
        <button @click="currentNumber++" class="mt-4 px-4 py-2 bg-blue-500 text-white rounded">Increase</button>
      </div>
    </div>
    <div v-else-if="viewState == ViewState.RoundResults">
      <div class="flex flex-col gap-4 w-full h-full">
        <div class="grid grid-cols-2 h-full w-full m-auto mt-16">
          <div class="flex justify-center">
            <span class="text-6xl flex justify-center mt-7 mr-6">Current round: </span>
            <span class="text-primary text-9xl flex justify-center">{{ payloadData.round }}</span>
          </div>
          <div class="flex justify-center">
            <span class="text-6xl flex justify-center mt-7 mr-6">Target: </span>
            <span class="text-primary text-9xl flex justify-center">{{ (payloadData.target / 1000).toFixed(0) }}s</span>
          </div>
        </div>
        <p class="text-4xl w-full text-center text-white mt-4">Round results:</p>
        <n-scrollbar class="-mb-4">
          <div class="grid gap-4">
            <div class="mx-auto mb-2 w-4/5" v-for="(player, i) in roundResultsData.results" :key="i">
              <n-card>
                <div class="w-full inline-flex justify-between text-2xl px-1">
                  <p class="inline-flex">
                    <span class="w-16">{{ i + 1 }}.</span
                    ><span class="font-bold col-span-5">{{ player.player }}</span>
                  </p>
                  <p>
                    <span v-if="player.diff == 0">Not pressed</span>
                    <span v-else>difference: <span class="font-bold">{{ (player.diff / 1000).toFixed(1) }}</span>s</span>
                  </p>
                </div>
              </n-card>
            </div>
          </div>
        </n-scrollbar>
      </div>
    </div>
    <div v-else-if="viewState == ViewState.Results">
      <div class="flex flex-col gap-4 w-full h-full">
        {{ resultsData }}
        <p class="text-4xl w-full text-center text-white mt-4">Round results:</p>
        <n-scrollbar class="-mb-4">
          <div class="grid gap-4">
            <div class="mx-auto mb-2 w-4/5" v-for="(player, i) in resultsData.results" :key="i">
              <n-card>
                <div class="w-full inline-flex justify-between text-2xl px-1">
                  <p class="inline-flex">
                    <span class="w-16">{{ i + 1 }}.</span
                    ><span class="font-bold col-span-5">{{ player.name }}</span>
                  </p>
                  <p>
                    <span v-if="player.round_one_diff == 0">Not pressed</span>
                    <span v-else>{{ (player.round_one_diff / 1000).toFixed(1) }}s</span>
                  </p>
                  <p>
                    <span v-if="player.round_two_diff == 0">Not pressed</span>
                    <span v-else>{{ (player.round_two_diff / 1000).toFixed(1) }}s</span>
                  </p>
                  <p>
                    <span v-if="player.round_three_diff == 0">Not pressed</span>
                    <span v-else>{{ (player.round_three_diff / 1000).toFixed(1) }}s</span>
                  </p>
                  <p>
                    <span v-if="player.average_diff_time == 0">Not pressed</span>
                    <span v-else class="font-bold">{{ (player.average_diff_time / 1000).toFixed(2) }}s</span>
                  </p>
                </div>
              </n-card>
            </div>
          </div>
        </n-scrollbar>
      </div>
    </div>
  </div>
</template>

<style scoped>
@keyframes fade-out {
  from {opacity: 100%;}
  to {opacity: 0%;}
}

.fade-out-button{
  animation: fade-out 3s forwards;
}
</style>