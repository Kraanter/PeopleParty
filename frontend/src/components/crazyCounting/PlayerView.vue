<script setup lang="ts">
import {
  CrazyCountingPlayerInputPayload,
  GameStateType,
  type CrazyCountingPlayerUpdatePayload,
  MiniGamePayloadType,
  Input,
  GameStatePayload
} from '@/flatbuffers/messageClass'
import { useWebSocketStore } from '@/stores/confettiStore'
import { defineProps, ref, toRefs, watch } from 'vue'
import { buildMessage } from '../../util/flatbufferMessageBuilder'
import * as flatbuffers from 'flatbuffers'

const props = defineProps<{
  width: number
  height: number
  data: MiniGamePayloadType
}>()

const websocketStore = useWebSocketStore()

const latestData = ref<CrazyCountingPlayerUpdatePayload>({} as CrazyCountingPlayerUpdatePayload)

const { data } = toRefs(props)

const proccessData = (data: MiniGamePayloadType) => {
  switch (data.gamestatetype()) {
    case GameStateType.CrazyCountingPlayerUpdate: {
      latestData.value = data.gamestatepayload(new CrazyCountingPlayerInputPayload())
    }
  }
}

watch(
  data,
  (newData) => {
    proccessData(newData)
  },
  { immediate: true }
)

const sendPlayerAction = (action: Input) => {
  let builder = new flatbuffers.Builder()

  let playerInput = CrazyCountingPlayerInputPayload.createCrazyCountingPlayerInputPayload(
    builder,
    action
  )

  let miniGamePayload = MiniGamePayloadType.createMiniGamePayloadType(
    builder,
    GameStateType.CrazyCountingPlayerInput,
    GameStatePayload.CrazyCountingPlayerInputPayload,
    playerInput
  )

  websocketStore.sendMessage(buildMessage(builder, miniGamePayload))
}
</script>
<template>
  <div class="w-full h-full flex">
    <div class="grid grid-rows-4 text-center my-auto max-w-full h-full gap-y-12 max-h-[75%] w-full">
      <div class="my-auto">
        <span class="text-8xl font-bold">{{ latestData.newInt() }}</span>
      </div>
      <div class="grid grid-cols-2 row-span-2 w-full h-full rounded-3xl overflow-hidden">
        <div
          v-if="latestData.timeLeft() <= 0"
          class="bg-green-500 col-span-2 text-center my-auto text-white text-4xl font-bold grid p-4 rounded-lg"
        >
          <span>Submitted</span>
        </div>
        <button
          v-else
          class="active:bg-opacity-100 bg-opacity-50 bg-red-500 border-r-2 border-dashed"
          @click="sendPlayerAction(Input.Decrease)"
          :disabled="latestData.newInt() <= 0"
        >
          <span class="text-8xl font-bold">-</span>
        </button>
        <button
          v-if="latestData.timeLeft() > 0"
          class="active:bg-opacity-100 bg-opacity-50 bg-green-500 border-l-2 border-dashed"
          @click="sendPlayerAction(Input.Increase)"
        >
          <span class="text-8xl font-bold">+</span>
        </button>
      </div>
      <button
        class="bg-red-400 active:bg-red-500 disabled:bg-slate-400 rounded-full text-white grid p-4"
        @click="sendPlayerAction(Input.Submit)"
        :disabled="latestData.newInt() <= 0"
      >
        <svg
          xmlns="http://www.w3.org/2000/svg"
          fill="none"
          viewBox="0 0 24 24"
          stroke-width="1.5"
          stroke="currentColor"
          class="w-full h-full"
        >
          <path
            stroke-linecap="round"
            stroke-linejoin="round"
            d="M16.5 10.5V6.75a4.5 4.5 0 1 0-9 0v3.75m-.75 11.25h10.5a2.25 2.25 0 0 0 2.25-2.25v-6.75a2.25 2.25 0 0 0-2.25-2.25H6.75a2.25 2.25 0 0 0-2.25 2.25v6.75a2.25 2.25 0 0 0 2.25 2.25Z"
          />
        </svg>
        <span class="text-2xl font-bold" v-if="latestData.newInt() <= 0">Anwser locked</span>
        <span class="text-2xl font-bold" v-else>Press to lock in</span>
        <span class="text-lg">{{ latestData.timeLeft() }} seconds left</span>
      </button>
    </div>
  </div>
</template>
