<script lang="ts" setup>
import { ref, toRefs, defineProps } from 'vue'
import TimeComponent from '../TimeComponent.vue'
import { type IntroductionData } from '@/components/introduction/Introduction.vue'
import { GameStateType, MemoryMixerGridPayload, MemoryMixerResultPayload, MemoryMixerRoundResultPayload, MiniGameIntroductionPayload, type MiniGamePayloadType } from '@/flatbuffers/messageClass';
import Introduction from '@/components/introduction/Introduction.vue'
import GridView from './GridView.vue'
import { processGrid, processMiniGameResult, processRoundResult, type MemoryMixerGrid, type MiniGameResult, type RoundResult } from './GridProcessor';
import { NCard } from 'naive-ui'

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

const grid = ref<MemoryMixerGrid>({
  timeLeft: 0,
  maxOnCard: -1,
  phase: -1,
  round: -1,
  active_players: -1,
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
  <div class="h-full">
    <div v-if="viewState == ViewState.Introduction">
      <Introduction :data="intro" logoSVG="/assets/games/memoryMixer/memoryMixerLogo.svg" />
    </div>
    <div v-else-if="viewState == ViewState.MiniGame || viewState == ViewState.RoundResults"
      class="h-full grid grid-cols-5">
      <div class="mt-4 ml-4 h-full col-span-2 col-start-1 flex flex-col">
        <div class="mx-auto mb-4">
          <div>
            <TimeComponent :time-left="grid.timeLeft" />
          </div>
        </div>
        <div class="text-center justify-center flex w-full mb-4 mt-4">
          <p class="text-4xl ml-4 mr-6 mt-2 text-white">Round {{ grid.round }}</p>
          <p class="text-4xl mr-4 ml-6 text-white"><span class="text-primary text-5xl"
              style="text-shadow: black 0px 0px">{{ grid.active_players }}</span> Players left</p>
        </div>
        <div v-if="grid.phase == 0">
          <p class="text-4xl w-full text-center text-white">Memorise the icons!</p>
        </div>
        <div v-else>
          <p class="text-4xl w-full text-center mt-8 text-white">Answers locked:</p>
          <n-scrollbar>
            <div class="mx-auto mt-4 mb-4 w-4/5" v-for="(name, i) in grid.submittedNames.slice().reverse()" :key="i">
              <n-card :class="[
      viewState == ViewState.RoundResults ? roundResult.correctNames.includes(name) ? ['outline', '-outline-offset-8', 'outline-8', 'outline-green-500'] : ['outline', '-outline-offset-8', 'outline-8', 'outline-red-500'] : ''
    ]">
                <p class="font-bold text-2xl w-full text-center overflow-ellipsis">
                  {{ name }}
                </p>
              </n-card>
            </div>
          </n-scrollbar>
        </div>
      </div>
      <div class="relative h-full col-span-3 col-start-3 justify-center items-center m-auto mt-16">
        <GridView :grid="grid" :player-submitted="{ playerSubmitted: false, x: -1, y: -1 }"
          :eliminated-players="eliminatedPlayers" :isHost="true">
        </GridView>
      </div>
    </div>
    <div v-else-if="viewState == ViewState.Results">
      <div class="flex flex-col gap-4 w-full h-full">
        <p class="text-4xl w-full text-center text-white mt-4">Final Ranking:</p>
        <n-scrollbar class="-mb-4">
          <div class="grid gap-4">
            <div class="mx-auto mb-2 w-4/5" v-for="(player, i) in miniGameResult.results" :key="i">
              <n-card>
                <div class="w-full inline-flex justify-between text-2xl px-1">
                  <p class="inline-flex">
                    <span class="w-16">{{ player.placement }}.</span><span class="font-bold col-span-5">{{ player.name
                      }}</span>
                  </p>
                  <p>
                    Rounds: <span class="font-bold">{{ player.rounds_won }}</span>
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
