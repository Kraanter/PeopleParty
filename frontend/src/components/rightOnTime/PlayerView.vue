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
  RightOnTimePayload,
} from '@/flatbuffers/messageClass'
import { type RightOnTimeData, type RightOnTimeResults, type RightOnTimeRoundResults } from './RightOnTimeModels'
import * as flatbuffers from 'flatbuffers'
import { buildMessage } from '@/util/flatbufferMessageBuilder'
import { useWebSocketStore } from '@/stores/confettiStore'
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

const submitted = computed(() => {
  return payloadData.value.submitted.includes(websocketStore.clientName)
})

const roundResultTime = ref<number>(0)

// final results
const resultsData = ref<RightOnTimeResults>({
  results: []
})

const update = (data: MiniGamePayloadType) => {
  switch (data.gamestatetype()) {
    case GameStateType.RightOnTime: {
      viewState.value = ViewState.MiniGame

      // do it this way so it doesnt overide the empty submitted array, because it will send empty from server
      const payload = parseRightOnTimePayload(data) 
      payloadData.value.fade_out = payload.fade_out
      payloadData.value.round = payload.round
      payloadData.value.target = payload.target
      payloadData.value.time = payload.time 
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

  payloadData.value.submitted.push(websocketStore.clientName)
  roundResultTime.value = payloadData.value.time
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
    <div class="flex h-full w-full justify-center items-center">
      <div class="mb-16 w-full">
        <div class="flex flex-col justify-center mt-8">
          <span class="text-4xl flex justify-center mb-2">Current round: </span>
          <span class="text-primary text-7xl flex justify-center">{{ payloadData.round }}</span>
        </div>
        <div class="grid grid-rows-1 grid-cols-1">
          <div class="flex justify-center mt-8 mb-16 relative overflow-visible tems-center h-full w-full">
            <button :disabled="submitted" @click="sendPlayerAction" class="eject-button">
              <span v-if="submitted">Locked</span>
              <span v-else>Submit</span>
            </button>
          </div>
        </div>
        <div class="grid grid-rows-2 grid-cols-2 justify-center mt-24">
          <span class="text-4xl flex justify-center mb-2" :class="{'col-span-2': !submitted}">Target: </span>
          <span class="text-4xl flex justify-center mb-2" v-if="submitted">Got: </span>
          <span class="text-primary text-7xl flex justify-center" :class="{'col-span-2': !submitted}">{{ payloadData.target / 1000 }}s</span>
          <span class="text-primary text-7xl flex justify-center" v-if="submitted">{{ roundResultTime / 1000 }}s</span>
        </div>
      </div>
    </div>
  </template>
  <template v-else-if="viewState == ViewState.Results">
    <div class="h-full w-full flex flex-cols mr-auto justify-center text-center mt-40">
      {{ resultsData.results }}
    </div>
  </template>
</template>

<style scoped>
.eject-button {
  background-color: red;
  color: white;
  border-radius: 50%;
  aspect-ratio: 1/1;
  --size: min(70vmin, 70vmax);
  max-width: var(--size);
  max-height: var(--size);
  line-height: 1;
  display: flex;
  justify-content: center;
  align-items: center;
  font-size: 8cap;
  box-shadow: 0rem 0.4em 0em 0.04em darkred;
  transition:
    box-shadow 0.3s,
    transform 0.3s !important;
}

.eject-button:disabled {
  background-color: slategray;
  transform: translateY(0.3em) !important;
  box-shadow: 0rem 0.1em 0 0.04em black !important;
}

.eject-button:active:not(:disabled) {
  transform: translateY(0.3em) !important;
  box-shadow: 0rem 0.1em 0 0.04em darkred !important;
}
</style>