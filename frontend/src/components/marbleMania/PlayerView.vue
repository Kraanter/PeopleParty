<script lang="ts" setup>
import { ref, defineProps, computed } from 'vue'
import TimeComponent from '../TimeComponent.vue'
import { type IntroductionData } from '@/components/introduction/Introduction.vue'
import {
  GameStateType,
  MiniGameIntroductionPayload,
  MiniGamePayloadType,
} from '@/flatbuffers/messageClass'
import { useWebSocketStore } from '@/stores/confettiStore'
import type { MarbleManiaPlayerData, MarbleManiaResult } from './MarbleManiaModels'
import { parseMarbleManiaPlayerPayload, parseMarbleManiaResultPayload } from './MarbleManiaProcessor'
import { sendPlayerAction, sendPlayerEvent } from '@/util/joystickMessageBuilder'
import JoystickComponent from '../highwayHustle/JoystickComponent.vue'
import { buildMessage } from '@/util/flatbufferMessageBuilder'
import {
  GameStatePayload,
  MarbleManiaPlayerInputPayload,
  MessageType,
  MiniGamePayloadType as MiniGamePayloadTypeBuilder,
  Payload,
} from '@/flatbuffers/messageClass'
import * as flatbuffers from 'flatbuffers'

const websocketStore = useWebSocketStore()

defineProps<{
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
  id: '',
  game_phase: 0, // 0 = placement, 1 = simulation,
  placement_time_left: 0,
  y_pos: 0,
  finish_line_y: 0,
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
    case GameStateType.MarbleManiaPlayer: {
      viewState.value = ViewState.MiniGame

      payloadData.value = parseMarbleManiaPlayerPayload(data)
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

const move = ({ x, y }: any) => {
  sendPlayerAction('marbleMania', x, y)
}

// Calculate marble progress as percentage (0-100) from start to finish
const getMarbleProgress = () => {
  if (!payloadData.value.finish_line_y) return 0
  
  // Assuming start is at a higher Y value (top) and finish is at lower Y (marble falls down)
  // Progress goes from 0% (start) to 100% (finish)
  const startY = -300  // World top boundary
  const finishY = payloadData.value.finish_line_y
  const currentY = payloadData.value.y_pos
  
  const totalDistance = finishY - startY
  const currentProgress = currentY - startY
  
  return Math.max(0, Math.min(100, (currentProgress / totalDistance) * 100))
}

// Send lock position command
const lockPosition = () => {
  const builder = new flatbuffers.Builder(1024)
  
  // Create MarbleManiaPlayerInputPayload
  const lock = true
  const playerInput = MarbleManiaPlayerInputPayload.createMarbleManiaPlayerInputPayload(
    builder,
    lock
  )
  
  const miniGame = builder.createString('marbleMania')
  
  const miniGamePayload = MiniGamePayloadTypeBuilder.createMiniGamePayloadType(
    builder,
    miniGame,
    GameStateType.MarbleManiaPlayerInput,
    GameStatePayload.MarbleManiaPlayerInputPayload,
    playerInput
  )
  
  // Send the message
  const message = buildMessage(builder, miniGamePayload, MessageType.MiniGame, Payload.MiniGamePayloadType)
  websocketStore.sendMessage(message)
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
      <div class="flex flex-col justify-center items-center mb-4">
        <div class="text-2xl text-white mb-2">Your Marble</div>
        <div class="text-lg text-white">{{ payloadData.id }}</div>
      </div>
      
      <div v-if="payloadData.game_phase == 0" class="flex flex-col justify-center items-center">
        <div class="text-xl text-white mb-4">Place Your Marble!</div>
        <div class="text-lg text-white mb-4">Time left: {{ Math.ceil(payloadData.placement_time_left) }}s</div>
        
        <!-- Joystick for marble placement -->
        <JoystickComponent
          class="no-project-style"
          style="margin: 20px"
          :size="200"
          base-color="lightgray"
          stick-color="blue"
          :throttle="50"
          @move="move"
          @start="sendPlayerEvent('marbleMania', 'start')"
          @stop="sendPlayerEvent('marbleMania', 'stop')"
        />
        
        <!-- Lock in button -->
        <button 
          @click="lockPosition"
          class="mt-4 px-8 py-4 bg-green-600 hover:bg-green-700 text-white text-xl font-bold rounded-lg"
        >
          Lock Position!
        </button>
      </div>
      
      <div v-else-if="payloadData.game_phase == 1" class="flex flex-col gap-4 w-full h-full justify-center items-center">
        <div class="flex flex-col justify-center items-center mb-4">
          <p class="text-2xl text-white">Your marble is rolling!</p>
        </div>
        
        <!-- Vertical progress bar -->
        <div class="flex flex-col items-center">
          <div class="text-white mb-2">Progress</div>
          <div class="relative w-8 h-64 bg-gray-700 rounded border-2 border-white">
            <!-- Finish line marker -->
            <div class="absolute w-full h-2 bg-red-500 top-0 rounded"></div>
            <div class="absolute -right-16 -top-1 text-white text-sm">Finish</div>
            
            <!-- Start marker -->
            <div class="absolute w-full h-2 bg-green-500 bottom-0 rounded"></div>
            <div class="absolute -right-12 -bottom-1 text-white text-sm">Start</div>
            
            <!-- Marble position indicator -->
            <div 
              class="absolute w-6 h-6 bg-blue-500 rounded-full border-2 border-white transform -translate-x-1"
              :style="{ bottom: `${getMarbleProgress()}%` }"
            ></div>
          </div>
        </div>
      </div>
      
      <div v-else-if="payloadData.game_phase == 2" class="flex flex-col justify-center items-center">
        <p class="text-2xl text-white">Marble finished!</p>
      </div>
    </div>
  </template>
  <template v-else-if="viewState == ViewState.Results">
    <div v-if="personalResult" class="flex flex-col gap-4 w-full h-full justify-center items-center">
      <div class="flex flex-col justify-center items-center">
        <div class="text-3xl mb-2">
          You got:
        </div>
        <div class="text-6xl m-4">
          {{ formatOrdinals(personalResult.placement) }}
        </div>
        <div v-if="personalResult.has_finished" class="mt-4 flex flex-col justify-center items-center">
          <div class="text-2xl">
            Finish Time:
          </div>
          <div class="text-4xl mt-2 bg-gray-600 p-2 rounded-2xl">
            {{ personalResult.time_to_finish.toFixed(2) }}s
          </div>
        </div>
        <div v-else class="mt-4 text-xl text-red-400">
          Did not finish
        </div>
      </div>
    </div>
  </template>
</template>

<style scoped>
</style>