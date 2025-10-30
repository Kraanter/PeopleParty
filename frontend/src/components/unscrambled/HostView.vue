<script lang="ts" setup>
import { ref, defineProps } from 'vue'
import { type IntroductionData } from '@/components/introduction/Introduction.vue'
import Introduction from '@/components/introduction/Introduction.vue'
import {
  GameStateType,
  MiniGameIntroductionPayload,
  type MiniGamePayloadType
} from '@/flatbuffers/messageClass'
import { type UnscrambledHostData, type UnscrambledResults, type UnscrambledRoundResults } from './UnscrambledModels'
import { NScrollbar, NCard } from 'naive-ui'
import { parseUnscrambledHostPayload, parseUnscrambledResults, parseUnscrambledRoundResults } from './UnscrambledProcessor'

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

const gameData = ref<UnscrambledHostData>({
  time: 0,
  round: 0,
  scrambled_word: '',
})

// round results
const roundResultsData = ref<UnscrambledRoundResults>({
  round: 0,
  correct: 0,
  results: []
})

// final results
const resultsData = ref<UnscrambledResults>({
  results: []
})

const update = (data: MiniGamePayloadType) => {
  switch (data.gamestatetype()) {
    case GameStateType.UnscrambledHost: {
      viewState.value = ViewState.MiniGame

      gameData.value = parseUnscrambledHostPayload(data)
      break
    }
    case GameStateType.UnscrambledRoundResult: {
      viewState.value = ViewState.RoundResults

      roundResultsData.value = parseUnscrambledRoundResults(data)
      break
    }
    case GameStateType.UnscrambledResult: {
      viewState.value = ViewState.Results

      resultsData.value = parseUnscrambledResults(data)
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
      <Introduction :data="intro" logoSVG="/assets/games/unscrambled/unscrambledLogo.svg" />
    </div>
    <div v-else-if="viewState == ViewState.MiniGame">
      <div class="flex h-full w-full justify-center items-center">
        <pre>{{ gameData }}</pre>
      </div>
    </div>
    <div v-else-if="viewState == ViewState.RoundResults">
      <div class="flex h-full w-full justify-center items-center">
        <pre>{{ roundResultsData }}</pre>
      </div>
    </div>
    <div v-else-if="viewState == ViewState.Results">
      <div class="flex h-full w-full justify-center items-center">
        <pre>{{ resultsData }}</pre>
      </div>
    </div>
  </div>
</template>