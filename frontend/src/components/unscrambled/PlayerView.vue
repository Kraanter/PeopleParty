<script lang="ts" setup>
import { ref, defineProps, computed } from 'vue'
import TimeComponent from '../TimeComponent.vue'
import { type IntroductionData } from '@/components/introduction/Introduction.vue'
import {
  GameStatePayload,
  GameStateType,
  MessageType,
  MiniGameIntroductionPayload,
  MiniGamePayloadType,
  Payload,
  UnscrambledPlayerInputPayload,
} from '@/flatbuffers/messageClass'
import type { UnscrambledRoundResults, UnscrambledPlayerData, UnscrambledResults } from './UnscrambledModels'
import * as flatbuffers from 'flatbuffers'
import { buildMessage } from '@/util/flatbufferMessageBuilder'
import { useWebSocketStore } from '@/stores/confettiStore'
import { NScrollbar, NCard } from 'naive-ui'
import { parseUnscrambledPlayerPayload, parseUnscrambledResults, parseUnscrambledRoundResults } from './UnscrambledProcessor'

const websocketStore = useWebSocketStore()

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

const gameData = ref<UnscrambledPlayerData>({
  time: 0,
  round: 0,
  words: [],
  submitted: false
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
    case GameStateType.UnscrambledPlayer: {
      viewState.value = ViewState.MiniGame

      gameData.value = parseUnscrambledPlayerPayload(data)
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

const sendPlayerAction = (index: number) => {
  let builder = new flatbuffers.Builder()

  let playerInput = UnscrambledPlayerInputPayload.createUnscrambledPlayerInputPayload(
    builder,
    index
  )

  let miniGame = builder.createString('Unscrambled')

  let miniGamePayload = MiniGamePayloadType.createMiniGamePayloadType(
    builder,
    miniGame,
    GameStateType.UnscrambledPlayerInput,
    GameStatePayload.UnscrambledPlayerInputPayload,
    playerInput
  )

  websocketStore.sendMessage(
    buildMessage(builder, miniGamePayload, MessageType.MiniGame, Payload.MiniGamePayloadType)
  )
}

defineExpose({
  update
})

</script>
<template>
  <template v-if="viewState == ViewState.Introduction">
    <div class="flex flex-col m-2 text-center gap-4 h-full justify-center items-center">
      <div class="w-full flex justify-center px-8">
        <div>
          <TimeComponent :timeLeft="intro.time_left" />
        </div>
      </div>
      <div>
        <div class="w-full h-full mt-16">
          <p class="text-4xl text-white">{{ intro.description }}</p>
        </div>
      </div>
    </div>
  </template>
  <template v-else-if="viewState == ViewState.MiniGame">
    <div class="flex flex-col h-full w-full justify-center items-center">
      <pre>{{ gameData }}</pre>

      <template v-for="(value, index) in gameData.words" :key="value.index">
        <button
          class="m-2 p-4 bg-blue-500 text-white rounded"
          :disabled="gameData.submitted"
          @click="sendPlayerAction(index)"
        >
          {{ value }}
        </button>
      </template>

    </div>
  </template>
  <template v-else-if="viewState == ViewState.RoundResults">
    <div class="flex h-full w-full justify-center items-center">
      <pre>{{ roundResultsData }}</pre>
    </div>
  </template>
  <template v-else-if="viewState == ViewState.Results">
    <div class="flex flex-col gap-4 w-full h-full">
      <pre>{{ resultsData }}</pre>
    </div>
  </template>
</template>