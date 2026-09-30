<script lang="ts" setup>
import { NCard, NScrollbar } from 'naive-ui'
import { ref, shallowRef } from 'vue'
import TimeComponent from '../TimeComponent.vue'
import {
  GameStateType,
  MiniGameIntroductionPayload,
  type MiniGamePayloadType
} from '@/flatbuffers/messageClass'
import { type IntroductionData } from '@/components/introduction/Introduction.vue'
import Introduction from '@/components/introduction/Introduction.vue'
import {
  parseCrazyCountingHostPayload,
  parseCrazyCountingResultPayload
} from './CrazyCountingProcessor'
import type { CrazyCountingEntity, CrazyCountingResult } from './CrazyCountingModels'
import CrazyCountingGameView from './CrazyCountingGameView.vue'

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
// game data
const entities = shallowRef<CrazyCountingEntity[]>([])
const timeLeft = ref<number>(0)
const submittedPlayers = ref<string[]>([])
// results
const results = ref<CrazyCountingResult>({
  correct_answer: 0,
  results: []
})

const update = (data: MiniGamePayloadType) => {
  switch (data.gamestatetype()) {
    case GameStateType.CrazyCountingHostEntities: {
      viewState.value = ViewState.MiniGame
      const parsed = parseCrazyCountingHostPayload(data)
      entities.value = parsed.entities
      timeLeft.value = parsed.time_left
      submittedPlayers.value = parsed.submitted
      break
    }
    case GameStateType.CrazyCountingResult: {
      viewState.value = ViewState.Results
      results.value = parseCrazyCountingResultPayload(data)
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
  <div v-if="viewState == ViewState.Introduction">
    <Introduction :data="intro" logoSVG="/assets/games/crazyCounting/crazyCountingLogo.svg" />
  </div>
  <div v-else-if="viewState == ViewState.MiniGame" class="flex justify-stretch w-full h-full">
    <div class="mt-4 w-full h-full flex flex-col justify-center">
      <div class="mx-auto mb-4">
        <TimeComponent :timeLeft />
      </div>
      <p class="text-4xl w-full text-center text-white">Answers locked:</p>
      <n-scrollbar class="mt-4">
        <div
          class="mx-auto mb-4 w-4/5"
          v-for="(name, i) in submittedPlayers.slice().reverse()"
          :key="i"
        >
          <n-card>
            <p class="font-bold text-2xl w-full text-center overflow-ellipsis">
              {{ name }}
            </p>
          </n-card>
        </div>
      </n-scrollbar>
    </div>
    <div class="relative w-full h-full">
      <CrazyCountingGameView :entities="entities" />
    </div>
  </div>
  <div v-else-if="viewState == ViewState.Results">
    <div class="flex flex-col gap-4 w-full h-full">
      <div class="flex">
        <p class="text-9xl w-full text-center m-auto text-primary">
          {{ results.correct_answer }}
        </p>
      </div>
      <p class="text-4xl w-full text-center text-white">Players guesses:</p>
      <n-scrollbar class="-mb-4">
        <div class="grid grid-cols-2 gap-4">
          <div class="mx-auto mb-2 w-4/5" v-for="(player, i) in results.results" :key="i">
            <n-card>
              <div class="w-full inline-flex justify-between text-2xl">
                <p class="inline-flex">
                  <span class="font-bold ml-4 col-span-5">{{ player.name }}</span>
                </p>
                <p>
                  <span class="font-bold">{{ player.guess }}</span>
                </p>
              </div>
            </n-card>
          </div>
        </div>
      </n-scrollbar>
    </div>
  </div>
</template>
