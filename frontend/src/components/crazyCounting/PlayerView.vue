<script setup lang="ts">
import {
  CrazyCountingPlayerInputPayload,
  CrazyCountingPlayerUpdatePayload,
  CrazyCountingResultPayload,
  GameStatePayload,
  GameStateType,
  Input,
  MessageType,
  MiniGameIntroductionPayload,
  MiniGamePayloadType,
  Payload
} from '@/flatbuffers/messageClass'
import { formatMilliseconds } from '@/util/funcs'
import { useWebSocketStore } from '@/stores/confettiStore'
import { computed, ref } from 'vue'
import { buildMessage } from '../../util/flatbufferMessageBuilder'
import * as flatbuffers from 'flatbuffers'
import PartyButton from '../PartyButton.vue'
import PeoplePartyLogo from '../PeoplePartyLogo.vue'
import type { IntroductionData } from '../introduction/Introduction.vue'
import TimeComponent from '../TimeComponent.vue'

defineProps<{
  width: number
  height: number
}>()

const websocketStore = useWebSocketStore()

interface latest {
  int: number
  timeLeft: number
  submitted: boolean
}

const latestData = ref<latest>({
  int: 0,
  timeLeft: 0,
  submitted: true
})

enum ViewState {
  None,
  Introduction,
  MiniGame,
  Results
}

const viewState = ref<ViewState>(ViewState.None)

const intro = ref<IntroductionData>({
  title: '',
  description: '',
  time_left: 0
})

const result = ref<number>(0)

const isDisabled = computed(
  () => latestData.value.submitted || viewState.value == ViewState.Results
)

const update = (data: MiniGamePayloadType) => {
  switch (data.gamestatetype()) {
    case GameStateType.CrazyCountingPlayerUpdate: {
      viewState.value = ViewState.MiniGame
      const newData: CrazyCountingPlayerUpdatePayload = data.gamestatepayload(
        new CrazyCountingPlayerUpdatePayload()
      )
      latestData.value = {
        int: newData.newInt(),
        timeLeft: Number(newData.timeLeft()),
        submitted: newData.submitted()
      }
      break
    }
    case GameStateType.MiniGameIntroduction: {
      viewState.value = ViewState.Introduction
      const newData: MiniGameIntroductionPayload = data.gamestatepayload(
        new MiniGameIntroductionPayload()
      )
      intro.value = {
        title: newData.name() || '',
        description: newData.instruction() || '',
        time_left: Number(newData.timeLeft())
      }
      break
    }
    case GameStateType.CrazyCountingResult: {
      viewState.value = ViewState.Results
      const newData: CrazyCountingResultPayload = data.gamestatepayload(
        new CrazyCountingResultPayload()
      )
      result.value = newData.correctAnswer()
      break
    }
  }
}

defineExpose({
  update
})

const sendPlayerAction = (action: Input) => {
  let builder = new flatbuffers.Builder()

  let playerInput = CrazyCountingPlayerInputPayload.createCrazyCountingPlayerInputPayload(
    builder,
    action
  )

  let miniGame = builder.createString('crazyCounting')

  let miniGamePayload = MiniGamePayloadType.createMiniGamePayloadType(
    builder,
    miniGame,
    GameStateType.CrazyCountingPlayerInput,
    GameStatePayload.CrazyCountingPlayerInputPayload,
    playerInput
  )

  websocketStore.sendMessage(
    buildMessage(builder, miniGamePayload, MessageType.MiniGame, Payload.MiniGamePayloadType)
  )
}
</script>
<template>
  <div
    v-if="viewState == ViewState.Introduction"
    class="flex flex-col m-2 text-center gap-4 h-full justify-center items-center"
  >
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
  <div
    v-else-if="viewState == ViewState.MiniGame || viewState == ViewState.Results"
    class="w-full h-full grid grid-cols-1 grid-rows-5 max-w-sm mx-auto"
  >
    <div class="w-full h-full flex justify-center items-center row-span-3 bg-black">
      <div
        style="box-shadow: inset 0.3em 0.3em var(--color-primary)"
        class="bg-slate-100 p-8 rounded-2xl h-3/4 w-full m-16"
      >
        <div class="flex flex-col items-center h-full justify-between">
          <div class="text-4xl flex text-center">
            <svg
              xmlns="http://www.w3.org/2000/svg"
              fill="none"
              viewBox="0 0 24 24"
              stroke-width="1.5"
              stroke="currentColor"
              class="w-6 h-6 mt-2 mr-4"
            >
              <path
                stroke-linecap="round"
                stroke-linejoin="round"
                d="M12 6v6h4.5m4.5 0a9 9 0 1 1-18 0 9 9 0 0 1 18 0Z"
              />
            </svg>
            {{ formatMilliseconds(latestData.timeLeft) }}
          </div>
          <div
            class="text-8xl"
            :class="{
              'diagonal-strike':
                viewState == ViewState.Results && Math.abs(result - latestData.int) != 0
            }"
          >
            {{ latestData.int }}
          </div>
          <div v-if="viewState == ViewState.Results">
            <div v-if="Math.abs(result - latestData.int) == 0">
              <p class="text-2xl">Good job, you are spot on!</p>
            </div>
            <div v-else>
              <p class="text-6xl text-primary" style="position: relative; top: -25px; left: 70px">
                {{ result }}
              </p>
              <p class="text-2xl">
                You were
                <span class="text-primary text-3xl">{{ Math.abs(result - latestData.int) }}</span>
                off
              </p>
            </div>
          </div>
          <div class="text-8xl">
            <svg
              xmlns="http://www.w3.org/2000/svg"
              viewBox="0 0 24 24"
              fill="currentColor"
              class="w-16 h-16"
            >
              <path
                v-if="latestData.submitted"
                fill-rule="evenodd"
                d="M12 1.5a5.25 5.25 0 0 0-5.25 5.25v3a3 3 0 0 0-3 3v6.75a3 3 0 0 0 3 3h10.5a3 3 0 0 0 3-3v-6.75a3 3 0 0 0-3-3v-3c0-2.9-2.35-5.25-5.25-5.25Zm3.75 8.25v-3a3.75 3.75 0 1 0-7.5 0v3h7.5Z"
                clip-rule="evenodd"
              />

              <path
                v-else
                d="M18 1.5c2.9 0 5.25 2.35 5.25 5.25v3.75a.75.75 0 0 1-1.5 0V6.75a3.75 3.75 0 1 0-7.5 0v3a3 3 0 0 1 3 3v6.75a3 3 0 0 1-3 3H3.75a3 3 0 0 1-3-3v-6.75a3 3 0 0 1 3-3h9v-3c0-2.9 2.35-5.25 5.25-5.25Z"
              />
            </svg>
          </div>
        </div>
      </div>
    </div>
    <div
      class="w-full h-full row-span-2 bg-secondary rounded-lg grid-cols-2 grid-rows-2 grid gap-4 p-8 pt-[20%]"
    >
      <div class="relative">
        <PartyButton
          :disabled="isDisabled"
          @click="sendPlayerAction(Input.Submit)"
          class="bg-sky-400 absolute h-2/3 w-full"
          >Lock</PartyButton
        >
      </div>
      <div class="relative">
        <div class="grid grid-cols-2 grid-rows-2">
          <PartyButton
            :disabled="isDisabled"
            @click="sendPlayerAction(Input.Increase)"
            class="rounded-full col-start-2 aspect-square"
            >+</PartyButton
          >
          <PartyButton
            :disabled="isDisabled"
            @click="sendPlayerAction(Input.Decrease)"
            class="rounded-full"
            >-</PartyButton
          >
        </div>
      </div>
      <PeoplePartyLogo />
    </div>
  </div>
</template>

<style scoped>
.diagonal-strike {
  position: relative;
  display: inline-block;
}

.diagonal-strike::after {
  content: '';
  position: absolute;
  top: 50%;
  left: 50%;
  width: 125%;
  min-width: 90px;
  height: 5px;
  background-color: red;
  transform: translate(-50%, -50%) rotate(-45deg);
  transform-origin: center center;
}
</style>
