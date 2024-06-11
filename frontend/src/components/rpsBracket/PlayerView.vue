<script setup lang="ts">
import { GameStateType } from '@/flatbuffers/game-state-type'
import {
  MessageType,
  Payload,
  GameStatePayload,
  MiniGameIntroductionPayload,
  MiniGamePayloadType,
  FB_RPSChoice,
  RPSBracketPlayerPayload,
  RPSBracketPlayerInputPayload
} from '@/flatbuffers/messageClass'
import { buildMessage } from '@/util/flatbufferMessageBuilder'
import TimeComponent from '../TimeComponent.vue'
import { Builder } from 'flatbuffers'
import { ref } from 'vue'
import { useWebSocketStore } from '@/stores/confettiStore'
import { NButton } from 'naive-ui'

enum ViewState {
  None,
  Introduction,
  MiniGame,
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
enum RPSChoice {
  None,
  Rock,
  Paper,
  Scissors,
}
interface playerDataPayload {
  choice: RPSChoice
  opponentChoice: RPSChoice
  opponentName: string
  winner: string
  time_left: number
}

// game data
const playerData = ref<playerDataPayload>({
  choice: RPSChoice.None,
  opponentChoice: RPSChoice.None,
  opponentName: '',
  winner: '',
  time_left: 0
})


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
    case GameStateType.RPSBracketPlayer: {
      viewState.value = ViewState.MiniGame
      const input: RPSBracketPlayerPayload = payload.gamestatepayload(new RPSBracketPlayerPayload())

      playerData.value = {
        choice: RPSChoice[input.choice() || 0] as unknown as RPSChoice,
        opponentChoice: RPSChoice[input.opponentChoice() || 0] as unknown as RPSChoice,
        opponentName: decodeURI(input.opponentName() || ''),
        winner: decodeURI(input.winner() || ''),
        time_left: Number(input.remainingTime())
      }
    }
  }
}

const webscoketStore = useWebSocketStore()

function player_action(action: FB_RPSChoice) {
  const builder = new Builder()

  let input = RPSBracketPlayerInputPayload.createRPSBracketPlayerInputPayload(
    builder,
    action
  )

  let miniGame = builder.createString('rpsBracket')

  let miniGamePayload = MiniGamePayloadType.createMiniGamePayloadType(
    builder,
    miniGame,
    GameStateType.RPSBracketPlayerInput,
    GameStatePayload.RPSBracketPlayerInputPayload,
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
    <div class="flex flex-col m-2 text-center gap-4 h-full justify-center items-center">
      <div v-if="playerData.winner != '' && playerData.winner == playerData.opponentName">
        <div class="w-full h-full mt-16">
          <p class="text-4xl text-white">You Lost</p>
          <p class="text-4xl text-white">You are elliminated from the game</p>
        </div>
      </div>
      <div v-else-if="playerData.winner != ''">
        <div class="w-full h-full mt-16">
          <p class="text-4xl text-white">You Won</p>
          <p class="text-4xl text-white">Waiting for opponent...</p>
        </div>
      </div>
      <div v-else class="w-full">
        <div>
          <p class="text-1xl">your name: {{  webscoketStore.clientName }}</p>
        </div>
        <div class="justify-center flex px-8 mb-16">
          <div>
            <TimeComponent :timeLeft="playerData.time_left" />
          </div>
        </div>
        <div>
          <div class="w-full mt-4">
            <p class="text-2xl text-white">opponentName {{ playerData.opponentName }}</p>
          </div>
        </div>
        <div>
          <div class="w-full mt-4">
            <p class="text-2xl text-white">opponent choice {{ playerData.opponentChoice }}</p>
          </div>
        </div>
        <div class="w-full mt-4">
          <p class="text-2xl text-white">your choise {{ playerData.choice }}</p>
        </div>
        <div class="w-full mt-4">
          <div class="flex flex-rows justify-center gap-4">
            <div>
              <n-button type="primary" size="large" @click="player_action(FB_RPSChoice.ROCK)">Rock</n-button>
            </div>
            <div>
              <n-button type="primary" size="large" @click="player_action(FB_RPSChoice.PAPER)">Paper</n-button>
            </div>
            <div>
              <n-button type="primary" size="large" @click="player_action(FB_RPSChoice.SCISSORS)">Scissors</n-button>
            </div>
          </div>
        </div>
      </div>
    </div>
  </template>
</template>
