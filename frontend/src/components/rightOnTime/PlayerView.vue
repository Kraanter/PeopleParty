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
import { type RightOnTimeData, type RightOnTimeResults } from './RightOnTimeModels'
import * as flatbuffers from 'flatbuffers'
import { buildMessage } from '@/util/flatbufferMessageBuilder'
import { useWebSocketStore } from '@/stores/confettiStore'
import { parseRightOnTimePayload, parseRightOnTimeResults } from './RightOnTimeProcessor'
import { NScrollbar, NCard } from 'naive-ui'

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

      payloadData.value = parseRightOnTimePayload(data)

      // if not pressed in time, lock the button and say not pressed in time
      if (payloadData.value.time >= payloadData.value.target + 9800) {
        if (roundResultTime.value === 0) {
          payloadData.value.submitted.push(websocketStore.clientName)
          roundResultTime.value = 0
        }
      }
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
          <span class="text-primary text-7xl flex justify-center" style="margin-top: -8px;" v-if="submitted">
            <span class="text-3xl flex flex-col justify-center items-center" v-if="roundResultTime == 0"><span>Not</span><span>pressed</span>(+{{ payloadData.target / 1000 }}s diff)</span>
            <span v-else>{{ roundResultTime / 1000 }}s</span>
          </span>
        </div>
      </div>
    </div>
  </template>
  <template v-else-if="viewState == ViewState.Results">
    <div class="flex flex-col gap-4 w-full h-full">
      <p class="text-4xl w-full text-center text-white mt-4">Minigame results:</p>
      <n-scrollbar class="-mb-4">
        <div class="grid gap-4">
          <div class="mx-auto mb-2 w-4/5">
            <n-card>
              <div class="w-full inline-flex justify-between text-2xl px-1">
                <p class="inline-flex ml-6">Name</p>
                <p>Average</p>
              </div>
            </n-card>
          </div>
          <div class="mx-auto mb-2 w-4/5" v-for="(player, i) in resultsData.results" :key="i">
            <n-card
              :style="
                player.name === websocketStore.clientName
                  ? 'background-color: var(--color-primary-dark); color: white'
                  : ''
              "
            >
              <div class="w-full inline-flex justify-between text-2xl px-1">
                <p class="inline-flex">
                  <span class="w-12">{{ i + 1 }}.</span
                  ><span class="font-bold col-span-5">{{ player.name }}</span>
                </p>
                <p>
                  <span class="font-bold">{{ (player.average_diff_time / 1000).toFixed(1) }}s</span>
                </p>
              </div>
            </n-card>
          </div>
        </div>
      </n-scrollbar>
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