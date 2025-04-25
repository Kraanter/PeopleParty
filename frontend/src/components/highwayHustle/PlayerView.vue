<script lang="ts" setup>
import { ref, defineProps, computed } from 'vue'
import TimeComponent from '../TimeComponent.vue'
import { type IntroductionData } from '@/components/introduction/Introduction.vue'
import {
  GameStatePayload,
  GameStateType,
  HighwayHustleResultPayload,
  JoystickDataPayload,
  JoystickEventPayload,
  JoystickEventType,
  MessageType,
  MiniGameIntroductionPayload,
  MiniGamePayloadType,
  Payload,
} from '@/flatbuffers/messageClass'
import * as flatbuffers from 'flatbuffers'
import { buildMessage } from '@/util/flatbufferMessageBuilder'
import { useWebSocketStore } from '@/stores/confettiStore'
import { NScrollbar, NCard } from 'naive-ui'
import type { HighwayHustlePlayerData, HighwayHustleResult } from './HighwayHustleModels'
import { parseHighwayHustlePlayerPayload } from './HighwayHustleProcessor'
import Joystick from 'vue-joystick-component'

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
const payloadData = ref<HighwayHustlePlayerData>({
  score: 0,
  isDead: false,
})

// minigame results
const results = ref<HighwayHustleResult>({
  results: []
})

const personalResult = computed(() => {
  return results.value.results.find((result) => result.name === websocketStore.clientName)
})

const update = (data: MiniGamePayloadType) => {
  switch (data.gamestatetype()) {
    case GameStateType.HighwayHustlePlayer: {
      viewState.value = ViewState.MiniGame

      payloadData.value = parseHighwayHustlePlayerPayload(data)
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
    case GameStateType.HighwayHustleResult: {
      viewState.value = ViewState.Results
      results.value = data.gamestatepayload(new HighwayHustleResultPayload())
      break;
    }
  }
  return []
}

const sendPlayerAction = (x: number, y: number) => {
  let builder = new flatbuffers.Builder()

  console.log('sending player action', x, y)
  let playerInput = JoystickDataPayload.createJoystickDataPayload(
    builder,
    x,
    y,
  )

  let miniGame = builder.createString('highwayHustle')

  let miniGamePayload = MiniGamePayloadType.createMiniGamePayloadType(
    builder,
    miniGame,
    GameStateType.JoystickData,
    GameStatePayload.JoystickDataPayload,
    playerInput
  )

  websocketStore.sendMessage(
    buildMessage(builder, miniGamePayload, MessageType.MiniGame, Payload.MiniGamePayloadType)
  )
}

const sendPlayerEvent = (event: string) => {
  console.log('sending player event', event)

  let builder = new flatbuffers.Builder()

  let eventType: JoystickEventType;
  switch (event) {
    case 'start':
      eventType = JoystickEventType.Start
      break
    case 'stop':
      eventType = JoystickEventType.Stop
      break
    default:
      eventType = JoystickEventType.Stop
      break
  }

  let playerInput = JoystickEventPayload.createJoystickEventPayload(
    builder,
    eventType,
  )

  let miniGame = builder.createString('highwayHustle')

  let miniGamePayload = MiniGamePayloadType.createMiniGamePayloadType(
    builder,
    miniGame,
    GameStateType.JoystickEvent,
    GameStatePayload.JoystickEventPayload,
    playerInput
  )

  websocketStore.sendMessage(
    buildMessage(builder, miniGamePayload, MessageType.MiniGame, Payload.MiniGamePayloadType)
  )
}

const move = ({ x, y }: any) => {
  sendPlayerAction(x, y)
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
      <div>
        {{ Math.round(payloadData.score / 10) }}
        {{ payloadData.isDead }}
      </div>
      <Joystick
        class="no-project-style"
        style="margin: 50px"
        :size="200"
        base-color="lightgray"
        stick-color="black"
        :throttle="100"
        @move="move"
        @start="sendPlayerEvent('start')"
        @stop="sendPlayerEvent('stop')"
      />
    </div>
  </template>
  <template v-else-if="viewState == ViewState.Results">
    <div v-if="personalResult" class="flex flex-col gap-4 w-full h-full">
      <!-- todo: mingame results -->
       <div>
        {{ personalResult.score }}
       </div>
       <div>
        You finished: {{ personalResult.placement }}
       </div>
    </div>
  </template>
</template>

<style scoped>
</style>