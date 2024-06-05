<script setup lang="ts">
import { GameStateType } from '@/flatbuffers/game-state-type'
import {
  MessageType,
  Payload,
  GameStatePayload,
  BusinessBailoutPlayerInputPayload,
  MiniGameIntroductionPayload,
  MiniGamePayloadType
} from '@/flatbuffers/messageClass'
import { buildMessage } from '@/util/flatbufferMessageBuilder'
import TimeComponent from '../TimeComponent.vue'
import { Builder } from 'flatbuffers'
import { parseBusinessBailoutPlayerPayload } from './parser'
import { ref } from 'vue'
import { useWebSocketStore } from '@/stores/confettiStore'
import MoneyCounter from './components/MoneyCounter.vue'

enum ViewState {
  None,
  Introduction,
  MiniGame,
  Result
}
const viewState = ref<ViewState>(ViewState.None)

type IntroductionData = {
  title: string
  description: string
  time_left: number
}
const intro = ref<IntroductionData>({
  title: '',
  description: '',
  time_left: 0
})

const value = ref(0)
const locked = ref(false)
const fromValue = ref(0)

function update(payload: MiniGamePayloadType) {
  switch (payload.gamestatetype()) {
    case GameStateType.MiniGameIntroduction: {
      viewState.value = ViewState.Introduction
      const introPayload: MiniGameIntroductionPayload = payload.gamestatepayload(
        new MiniGameIntroductionPayload()
      )
      intro.value = {
        title: introPayload.name() || '',
        description: introPayload.instruction() || '',
        time_left: Number(introPayload.timeLeft())
      }
      break
    }
    case GameStateType.BusinessBailoutPlayer: {
      viewState.value = ViewState.MiniGame
      const { value: newValue, locked: newLocked } = parseBusinessBailoutPlayerPayload(payload)

      fromValue.value = value.value
      value.value = newValue
      locked.value = newLocked
    }
  }
}

const webscoketStore = useWebSocketStore()

function click() {
  const builder = new Builder()

  let input = BusinessBailoutPlayerInputPayload.createBusinessBailoutPlayerInputPayload(
    builder,
    true
  )

  let miniGame = builder.createString('businessBailout')

  let miniGamePayload = MiniGamePayloadType.createMiniGamePayloadType(
    builder,
    miniGame,
    GameStateType.BusinessBailoutPlayerInput,
    GameStatePayload.BusinessBailoutPlayerInputPayload,
    input
  )

  webscoketStore.sendMessage(
    buildMessage(builder, miniGamePayload, MessageType.MiniGame, Payload.MiniGamePayloadType)
  )
}

defineExpose({ update })
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
  <template v-else-if="viewState === ViewState.MiniGame">
    <div class="grid grid-rows-3 grid-cols-1 h-full w-full p-2">
      <div class="w-full h-full flex justify-center items-center">
        <div
          class="bg-slate-100 p-8 flex justify-center items-center rounded-2xl h-3/4 w-full my-8"
        >
          <MoneyCounter :value />
        </div>
      </div>
      <div class="flex justify-center relative overflow-clip tems-center row-span-2 h-full w-full">
        <button :disabled="locked" @click="click" class="eject-button">
          <span v-if="locked">Locked</span>
          <span v-else>Sell</span>
        </button>
      </div>
    </div>
  </template>
</template>
<style scoped>
.eject-button {
  background-color: red;
  color: white;
  border-radius: 50%;
  aspect-ratio: 1/1;
  --size: min(80vmin, 80vmax);
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
  box-shadow: 0rem 0.4rem 0 0.04em black !important;
  transition: none;
}

.eject-button:active {
  transform: translate(0rem, 0.4rem) !important;
  box-shadow: 0rem 0.4rem 0 0.04em darkred !important;
}
</style>
