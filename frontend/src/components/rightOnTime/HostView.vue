<script lang="ts" setup>
import { ref, defineProps } from 'vue'
import { type IntroductionData } from '@/components/introduction/Introduction.vue'
import Introduction from '@/components/introduction/Introduction.vue'
import {
  GameStateType,
  RightOnTimePayload,
  RightOnTimeResultPayload,
  RightOnTimeRoundResultPayload,
  MiniGameIntroductionPayload,
  type MiniGamePayloadType
} from '@/flatbuffers/messageClass'
import {
  type RightOnTimeData,
  type RightOnTimeRoundResults,
  type RightOnTimeResults,
  type RightOnTimeRoundResultPair,
  type RightOnTimeResultPair,
} from './RightOnTimeModels'
import { NScrollbar, NCard } from 'naive-ui'
import { parseRightOnTimePayload, parseRightOnTimeResults, parseRightOnTimeRoundResults } from './RightOnTimeProcessor'

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
      viewState.value = ViewState.Results

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
</script>
<template>
  <div class="h-full">
    <div v-if="viewState == ViewState.Introduction">
      <Introduction :data="intro" logoSVG="/assets/games/rightOnTime/rightOnTimeLogo.svg" />
    </div>
    <div v-else-if="viewState == ViewState.MiniGame">
      <div class="h-full w-full flex flex-col justiy-items text-center m-auto mt-16">
        {{ payloadData }}
      </div>
    </div>
    <div v-else-if="viewState == ViewState.RoundResults">
      <div class="flex flex-col gap-4 w-full h-full">
        {{ roundResultsData }}
      </div>
    </div>
    <div v-else-if="viewState == ViewState.Results">
      <div class="flex flex-col gap-4 w-full h-full">
        {{ resultsData }}
      </div>
    </div>
  </div>
</template>
