<script lang="ts" setup>
import { ref, toRefs, defineProps } from 'vue'
import { type IntroductionData } from '@/components/introduction/Introduction.vue'
import { 
  GameStatePayload,
  GameStateType, 
  MemoryMixerGridPayload, 
  MemoryMixerPlayerInputPayload, 
  MemoryMixerPlayerSubmittedPayload, 
  MemoryMixerResultPayload, 
  MemoryMixerRoundResultPayload, 
  MessageType, 
  MiniGameIntroductionPayload, 
  MiniGamePayloadType, 
  Payload 
} from '@/flatbuffers/messageClass';
import TimeComponent from '../TimeComponent.vue'
import GridView from './GridView.vue'
import { processGrid, processRoundResult, type MemoryMixerGrid, type PlayerSubmittedData, type MiniGameResult, type RoundResult, processMiniGameResult } from './GridProcessor';
import * as flatbuffers from 'flatbuffers'
import { useWebSocketStore } from '@/stores/confettiStore';
import { buildMessage } from '@/util/flatbufferMessageBuilder';
import { NCard } from 'naive-ui'
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
  RoundResults,
  Results
}

const viewState = ref<ViewState>(ViewState.None)

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
  round: -1,
  active_players: -1,
  submittedNames: [],
  grid: [],
})

//round result
const roundResult = ref<RoundResult>({
  round: -1,
  correctNames: [],
  wrongNames: [],
})
const eliminatedPlayers = ref<string[]>([])

// if player has submitted
const playerSubmitted = ref<PlayerSubmittedData>({
  playerSubmitted: false,
  x: -1,
  y: -1
})

// minigame result
const miniGameResult = ref<MiniGameResult>({
  round: -1,
  results: []
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
    case GameStateType.MemoryMixerRoundResult: {
      viewState.value = ViewState.RoundResults
      const roundResultPayload: MemoryMixerRoundResultPayload = data.gamestatepayload(
        new MemoryMixerRoundResultPayload()
      )

      roundResult.value = processRoundResult(roundResultPayload)

      eliminatedPlayers.value.push(...roundResult.value.wrongNames)
      
      return roundResultPayload
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
    case GameStateType.MemoryMixerResult: {
      viewState.value = ViewState.Results
      const miniGameResultPayload: MemoryMixerResultPayload = data.gamestatepayload(new MemoryMixerResultPayload())

      miniGameResult.value = processMiniGameResult(miniGameResultPayload)

      return null
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
          <div class="w-full h-full mt-4">
              <p class="text-4xl text-white">{{ intro.description }}</p>
          </div>
      </div>
  </div>
  <div v-else-if="viewState == ViewState.MiniGame || viewState == ViewState.RoundResults" class="flex justify-stretch">
    <div class="w-full h-screen flex flex-col justify-center items-center">
      <div v-if="eliminatedPlayers.includes(websocketStore.clientName)">
        <n-card class="mb-4">
            <p class="font-bold text-2xl w-full text-center overflow-ellipsis">
              You have been eliminated :&#40; <br>Better luck next time!
            </p>
        </n-card>
      </div>
      <div v-else>
        <div class="w-full flex justify-center px-8">
        <div class="mx-auto mb-8">
          <TimeComponent :time-left="grid.timeLeft" />
        </div>
      </div>
      <GridView :grid="grid" :player-submitted="playerSubmitted" :eliminated-players="eliminatedPlayers" :isHost="false" @click="sendPlayerAction"></GridView>
      </div>
      <div class="h-48 w-48 -mt-8">
        <PeoplePartyLogo />
      </div>
    </div>
  </div>
  <div v-else-if="viewState == ViewState.Results">
    <div class="flex flex-col gap-4 w-full h-full">
      <p class="text-4xl w-full text-center text-white mt-4">Final Ranking:</p>
      <n-scrollbar class="-mb-4">
        <div class="grid gap-4">
          <div class="mx-auto mb-2 w-4/5" v-for="(player, i) in miniGameResult.results" :key="i">
            <n-card 
            :style="
            player.name === websocketStore.clientName
              ? 'background-color: var(--color-primary-dark); color: white'
              : ''
            ">
              <div class="w-full inline-flex justify-between text-2xl px-1">
                <p class="inline-flex">
                  <span class="w-16">{{ player.placement }}.</span
                  ><span class="font-bold col-span-5">{{ player.name }}</span>
                </p>
                <p>
                  Rounds: <span class="font-bold">{{ player.rounds_won }}</span>
                </p>
              </div>
            </n-card>
          </div>
        </div>
      </n-scrollbar>
    </div>
  </div>
</template>
