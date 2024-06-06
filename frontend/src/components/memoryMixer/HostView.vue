<script lang="ts" setup>
import { ref, toRefs, defineProps } from 'vue'
import TimeComponent from '../TimeComponent.vue'
import { type IntroductionData } from '@/components/introduction/Introduction.vue'
import { GameStateType, MemoryMixerGridPayload, MemoryMixerResultPayload, MemoryMixerRoundResultPayload, MiniGameIntroductionPayload, type MiniGamePayloadType } from '@/flatbuffers/messageClass';
import Introduction from '@/components/introduction/Introduction.vue'
import GridView from './GridView.vue'
import { processGrid, processMiniGameResult, processRoundResult, type MemoryMixerGrid, type MiniGameResult, type RoundResult } from './GridProcessor';


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

const { width, height } = toRefs(props)

const grid = ref<MemoryMixerGrid>({
  timeLeft: 0,
  maxOnCard: -1,
  phase: -1,
  round: -1,
  submittedNames: [],
  grid: [],
})

//round result
const roundResult = ref<RoundResult>({
  round: -1,
  correctNames: [],
  wrongNames: [],
})
const eliminatedPlayers = ref<string[]>([])

// introduction
const intro = ref<IntroductionData>({
  title: '',
  description: '',
  time_left: 0
})

// minigame result
const miniGameResult = ref<MiniGameResult>({
  round: -1,
  results: []
})

const update = (data: MiniGamePayloadType) => {
  switch (data.gamestatetype()) {
    case GameStateType.MemoryMixerGrid: {
      viewState.value = ViewState.MiniGame
      const hostEntitiesPayload: MemoryMixerGridPayload = data.gamestatepayload(
        new MemoryMixerGridPayload()
      )

      grid.value = processGrid(hostEntitiesPayload)
      return null;
    }
    case GameStateType.MemoryMixerRoundResult: {
      viewState.value = ViewState.RoundResults
      const roundResultPayload: MemoryMixerRoundResultPayload = data.gamestatepayload(
        new MemoryMixerRoundResultPayload()
      )

      roundResult.value = processRoundResult(roundResultPayload)

      eliminatedPlayers.value.push(...roundResult.value.wrongNames)
      
      return roundResultPayload
    }
    case GameStateType.MemoryMixerResult: {
      viewState.value = ViewState.Results
      const miniGameResultPayload: MemoryMixerResultPayload = data.gamestatepayload(new MemoryMixerResultPayload())

      miniGameResult.value = processMiniGameResult(miniGameResultPayload)

      return null
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
      return intro.value
    }
  }
  return []
}

defineExpose({
  update
})
</script>
<template>
  <div v-if="viewState == ViewState.RoundResults">
    <p class="text-2xl">round results of round {{ roundResult.round }}</p>
    <span>correct guesses {{ roundResult.correctNames }}</span><br>
    <span>wrong guesses {{ roundResult.wrongNames }}</span>
  </div>
  <div v-else-if="viewState == ViewState.Introduction">
    <Introduction :data="intro" logoSVG="/assets/games/memoryMixer/memoryMixerLogo.svg" />
  </div>
  <div v-else-if="viewState == ViewState.MiniGame" class="flex justify-stretch">
    <span>round {{ grid.round }}</span><br>
    <span>submitted names {{ grid.submittedNames }}</span>
    <div class="mt-4 w-full h-full flex flex-col justify-center items-center">
        <div class="mx-auto mb-4">
          <TimeComponent :time-left="grid.timeLeft" />
        </div>
        <div class="mx-auto mt-4">
          <GridView 
          :grid="grid" 
          :player-submitted="{playerSubmitted: false, x: -1, y: -1}"
          :eliminated-players="eliminatedPlayers"
          :isHost="true">
          </GridView>
        </div>
      </div>
  </div>
  <div v-else-if="viewState == ViewState.Results">
    {{ miniGameResult }}
  </div>
</template>
