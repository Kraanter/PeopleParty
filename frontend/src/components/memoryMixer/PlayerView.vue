<script lang="ts" setup>
import { ref, toRefs, defineProps } from 'vue'
import { type IntroductionData } from '@/components/introduction/Introduction.vue'
import { 
  GameStatePayload,
  GameStateType, 
  MemoryMixerGridPayload, 
  MemoryMixerPlayerInputPayload, 
  MemoryMixerPlayerSubmittedPayload, 
  MessageType, 
  MiniGameIntroductionPayload, 
  MiniGamePayloadType, 
  Payload 
} from '@/flatbuffers/messageClass';
import TimeComponent from '../TimeComponent.vue'
import GridView from './GridView.vue'
import { processGrid, type MemoryMixerGrid, type PlayerSubmittedData } from './GridProcessor';
import * as flatbuffers from 'flatbuffers'
import { useWebSocketStore } from '@/stores/confettiStore';
import { buildMessage } from '@/util/flatbufferMessageBuilder';
import PeoplePartyLogo from '../PeoplePartyLogo.vue'

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

const { width, height } = toRefs(props)

// introduction
const intro = ref<IntroductionData>({
  title: '',
  description: '',
  time_left: 0
})
// game data
const grid = ref<MemoryMixerGrid>({
  timeLeft: 0,
  maxOnCard: -1,
  phase: -1,
  grid: [],
})
const timeLeft = ref<number>(0)
// if player has submitted
const playerSubmitted = ref<PlayerSubmittedData>({
  playerSubmitted: false,
  x: -1,
  y: -1
})

const update = (data: MiniGamePayloadType) => {
  switch (data.gamestatetype()) {
    case GameStateType.MemoryMixerGrid: {
      viewState.value = ViewState.MiniGame
      const hostEntitiesPayload: MemoryMixerGridPayload = data.gamestatepayload(
        new MemoryMixerGridPayload()
      )

      grid.value = processGrid(hostEntitiesPayload)
      return null;
    }
    case GameStateType.MemoryMixerPlayerSubmitted: {
      const playerSumbitted: MemoryMixerPlayerSubmittedPayload = data.gamestatepayload(
        new MemoryMixerPlayerSubmittedPayload()
      )

      playerSubmitted.value = {
        playerSubmitted: playerSumbitted.submitted(),
        x: playerSumbitted.x(),
        y: playerSumbitted.y()
      }

      return null;
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
      return intro.value
    }
  }
  return []
}

const sendPlayerAction = (x: number, y: number) => {
  let builder = new flatbuffers.Builder()

  let playerInput = MemoryMixerPlayerInputPayload.createMemoryMixerPlayerInputPayload(
    builder,
    x,
    y
  )

  let miniGame = builder.createString('memoryMixer')

  let miniGamePayload = MiniGamePayloadType.createMiniGamePayloadType(
    builder,
    miniGame,
    GameStateType.MemoryMixerPlayerInput,
    GameStatePayload.MemoryMixerPlayerInputPayload,
    playerInput
  )

  websocketStore.sendMessage(
    buildMessage(builder, miniGamePayload, MessageType.MiniGame, Payload.MiniGamePayloadType)
  )
}

defineExpose({
  update
})
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
  <div v-else-if="viewState == ViewState.MiniGame" class="flex justify-stretch">
    <div class="mt-4 w-full h-screen flex flex-col justify-center items-center">
      <GridView :grid="grid" :player-submitted="playerSubmitted" :isHost="false" @click="sendPlayerAction"></GridView>
    </div>
  </div>
  <div v-else-if="viewState == ViewState.Results">

  </div>
</template>
