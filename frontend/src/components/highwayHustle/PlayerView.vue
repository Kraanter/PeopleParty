<script lang="ts" setup>
import { ref, defineProps, computed } from 'vue'
import TimeComponent from '../TimeComponent.vue'
import { type IntroductionData } from '@/components/introduction/Introduction.vue'
import {
  GameStatePayload,
  GameStateType,
  JoystickDataPayload,
  MessageType,
  MiniGameIntroductionPayload,
  MiniGamePayloadType,
  Payload,
} from '@/flatbuffers/messageClass'
import * as flatbuffers from 'flatbuffers'
import { buildMessage } from '@/util/flatbufferMessageBuilder'
import { useWebSocketStore } from '@/stores/confettiStore'
import { NScrollbar, NCard } from 'naive-ui'
import type { HighwayHustlePlayerData } from './HighwayHustleModels'
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
})

// minigame results


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
    <div class="flex h-full w-full justify-center items-center">
      <Joystick
        class="no-project-style"
        style="margin: 50px"
        :size="200"
        base-color="lightgray"
        stick-color="black"
        :throttle="100"
        @move="move"
      />
    </div>
  </template>
  <template v-else-if="viewState == ViewState.Results">
    <div class="flex flex-col gap-4 w-full h-full">
      <!-- todo: mingame results -->
    </div>
  </template>
</template>

<style scoped>
/* :deep(button), button {
  box-shadow: 0.0rem 0.0rem 0 #000 !important;
  transition:
    transform 0s ease-in-out,
    box-shadow 0s ease-in-out !important;
  
}

:deep(button):active, button {
  transform: translate(0.0rem, 0.0rem) !important;
  box-shadow: 0.0rem 0.0rem 0 #000 !important;
} */


/* button {
  box-shadow: 0.5rem 0.5rem 0 #000;
  transition:
    transform 0.1s ease-in-out,
    box-shadow 0.1s ease-in-out !important;
}

button:active {
  transform: translate(0.5rem, 0.5rem) !important;
  box-shadow: 0.1rem 0.1rem 0 #000 !important;
} */



/* Button: {
    iconSizeLarge: '1.7rem',
    borderRadiusLarge: '2rem'
} */
</style>