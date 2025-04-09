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
import Joystick from '@mazi1/joystick'

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
        style="margin: 50px"
        :size="100"
        base-color="pink"
        stick-color="purple"
        :throttle="100"
        @start="null"
        @stop="null"
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