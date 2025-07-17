<script lang="ts" setup>
import { ref, defineProps, computed } from 'vue'
import TimeComponent from '../TimeComponent.vue'
import { type IntroductionData } from '@/components/introduction/Introduction.vue'
import {
  GameStatePayload,
  GameStateType,
  MarbleManiaPlayerInputPayload,
  MessageType,
  MiniGameIntroductionPayload,
  MiniGamePayloadType,
  Payload,
} from '@/flatbuffers/messageClass'
import * as flatbuffers from 'flatbuffers'
import { buildMessage } from '@/util/flatbufferMessageBuilder'
import { useWebSocketStore } from '@/stores/confettiStore'
import type { MarbleManiaPlayerData, MarbleManiaResult } from './MarbleManiaModels'
import { parseMarbleManiaPlayerInputPayload, parseMarbleManiaResultPayload } from './MarbleManiaProcessor'

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

// player payload data
const payloadData = ref<MarbleManiaPlayerData>({
  x_pos: 0,
  y_pos: 0,
  action_type: 0, // 0 = place_marble, 1 = ready_up
})

// minigame results
const results = ref<MarbleManiaResult>({
  results: []
})

const personalResult = computed(() => {
  return results.value.results.find((result) => result.name === websocketStore.clientName)
})

const update = (data: MiniGamePayloadType) => {
  switch (data.gamestatetype()) {
    case GameStateType.MarbleManiaPlayerInput: {
      viewState.value = ViewState.MiniGame

      payloadData.value = parseMarbleManiaPlayerInputPayload(data)
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
    case GameStateType.MarbleManiaResult: {
      viewState.value = ViewState.Results
      results.value = parseMarbleManiaResultPayload(data)
      break;
    }
  }
  return []
}

const sendPlayerAction = (x: number, y: number) => {
  let builder = new flatbuffers.Builder()

  let playerInput = MarbleManiaPlayerInputPayload.createMarbleManiaPlayerInputPayload(
    builder,
    x,
    y,
    payloadData.value.action_type, // 0 = place_marble, 1 = ready_up
  )

  let miniGame = builder.createString('marbleMania')

  let miniGamePayload = MiniGamePayloadType.createMiniGamePayloadType(
    builder,
    miniGame,
    GameStateType.MarbleManiaPlayerInput,
    GameStatePayload.MarbleManiaPlayerInputPayload,
    playerInput
  )

  websocketStore.sendMessage(
    buildMessage(builder, miniGamePayload, MessageType.MiniGame, Payload.MiniGamePayloadType)
  )
}

const pr = new Intl.PluralRules('en-US', { type: 'ordinal' })
const suffixes = new Map([
  ['one', 'st'],
  ['two', 'nd'],
  ['few', 'rd'],
  ['other', 'th']
])
const formatOrdinals = (n: number) => {
  const rule = pr.select(n)
  const suffix = suffixes.get(rule)
  return `${n}${suffix}`
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
      
    </div>
  </template>
  <template v-else-if="viewState == ViewState.Results">
    <div v-if="personalResult" class="flex flex-col gap-4 w-full h-full justify-center items-center">
      <div class="flex flex-col justify-center items-center">
        
      </div>
    </div>
  </template>
</template>

<style scoped>
</style>