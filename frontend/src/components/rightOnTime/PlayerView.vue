<script lang="ts" setup>
import { ref, defineProps } from 'vue'
import TimeComponent from '../TimeComponent.vue'
import { type IntroductionData } from '@/components/introduction/Introduction.vue'
import {
  GameStatePayload,
  GameStateType,
  MessageType,
  MiniGameIntroductionPayload,
  MiniGamePayloadType,
  Payload,
  RightOnTimePayload,
  RightOnTimeResultPayload,
  RightOnTimeRoundResultPayload,
} from '@/flatbuffers/messageClass'
import { type RightOnTimeData, type RightOnTimeResultPair, type RightOnTimeResults, type RightOnTimeRoundResultPair, type RightOnTimeRoundResults } from './RightOnTimeModels'
import * as flatbuffers from 'flatbuffers'
import { buildMessage } from '@/util/flatbufferMessageBuilder'
import { useWebSocketStore } from '@/stores/confettiStore'
import PartyButton from '@/components/PartyButton.vue'
import { parseRightOnTimePayload, parseRightOnTimeResults, parseRightOnTimeRoundResults } from './RightOnTimeProcessor'

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

      roundResultsData.value = parseRightOnTimeRoundResults(data, websocketStore.clientName)
      break
    }
    case GameStateType.RightOnTimeResult: {
      viewState.value = ViewState.Results

      resultsData.value = parseRightOnTimeResults(data, websocketStore.clientName)
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

const sendPlayerAction = () => {
  let builder = new flatbuffers.Builder()

  let playerInput = RightOnTimePayload.createRightOnTimePayload(
    builder,
    payloadData.value.round,
    payloadData.value.target,
    BigInt(payloadData.value.time),
    payloadData.value.fade_out,
    0
  )

  let miniGame = builder.createString('rightOnTime')

  let miniGamePayload = MiniGamePayloadType.createMiniGamePayloadType(
    builder,
    miniGame,
    GameStateType.RightOnTime,
    GameStatePayload.RightOnTimePayload,
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
    <div class="mt-32">
      {{ payloadData.round }}
      {{ payloadData.target }}
      {{ payloadData.time }}
      {{ payloadData.fade_out }}
      <PartyButton @click="sendPlayerAction"> 
        <p class="text-4xl m-6">Submit</p>
      </PartyButton>
    </div>
  </template>
  <template v-else-if="viewState == ViewState.RoundResults">
    <div class="mt-32">
      {{ roundResultsData.round }}
      {{ roundResultsData.target }}
      {{ roundResultsData.results }}
    </div>
  </template>
  <template v-else-if="viewState == ViewState.Results">
    <div class="h-full w-full flex flex-cols mr-auto justify-center text-center mt-40">
      {{ resultsData.results }}
    </div>
  </template>
</template>
