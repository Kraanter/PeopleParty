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
import { NCard } from 'naive-ui'
import PartyButton from '../PartyButton.vue'

enum ViewState {
  None,
  Introduction,
  MiniGame
}
const viewState = ref<ViewState>(ViewState.MiniGame)

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
  None = 0,
  Rock = 1,
  Paper = 2,
  Scissors = 3
}

const RPSChoiceMap = {
  [RPSChoice.Rock]: 'Rock',
  [RPSChoice.Paper]: 'Paper',
  [RPSChoice.Scissors]: 'Scissors'
}

interface playerDataPayload {
  choice: RPSChoice
  opponentName: string
  winner: string
  time_left: number
}

// game data
const playerData = ref<playerDataPayload>({
  choice: RPSChoice.None,
  opponentName: '[Enemy name]',
  winner: '',
  time_left: 10000
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
        opponentName: decodeURI(input.opponentName() || ''),
        winner: decodeURI(input.winner() || ''),
        time_left: Number(input.remainingTime())
      }
    }
  }
}

const webscoketStore = useWebSocketStore()

const FB_RPSChoiceMap = {
  [RPSChoice.Rock]: FB_RPSChoice.ROCK,
  [RPSChoice.Paper]: FB_RPSChoice.PAPER,
  [RPSChoice.Scissors]: FB_RPSChoice.SCISSORS
}

function player_action(action: RPSChoice) {
  playerData.value.choice = action

  if (action === RPSChoice.None) return
  const builder = new Builder()

  let input = RPSBracketPlayerInputPayload.createRPSBracketPlayerInputPayload(
    builder,
    FB_RPSChoiceMap[action]
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
        <div class="mt-4 absolute aspect-square top-0 right-1 left-1">
          <n-card class="text-2xl text-white">
            <p class="text-xl mb-4">Selected weapon:</p>
            <div v-if="playerData.choice">
              <img
                class="m-auto size-32 aspect-square"
                :src="`/assets/games/rpsBracket/${RPSChoiceMap[playerData.choice].toLowerCase()}.svg`"
              />
              {{ RPSChoiceMap[playerData.choice] }}
            </div>
          </n-card>
        </div>
        <!-- <div>
          <p class="text-1xl">your name: {{ webscoketStore.clientName }}</p>
        </div> -->
        <div>
          <div class="w-full mt-4">
            <p class="text-2xl text-white">Playing against: {{ playerData.opponentName }}</p>
          </div>
        </div>
        <div class="w-full mt-4 px-4">
          <div class="grid grid-cols-3 gap-4">
            <div v-for="(choice, i) in RPSChoiceMap" :key="choice">
              <PartyButton
                class="m-2 !text-lg"
                :class="{
                  'bg-sky-400': i === playerData.choice,
                  'bg-primary': i !== playerData.choice
                }"
                @click="player_action(i)"
              >
                <img
                  class="size-12 m-auto"
                  :src="`/assets/games/rpsBracket/${choice.toLowerCase()}.svg`"
                />
                {{ choice }}
              </PartyButton>
            </div>
          </div>
        </div>
      </div>
      <div class="absolute bottom-0 px-8 mb-16">
        <div>
          <TimeComponent :timeLeft="playerData.time_left" />
        </div>
      </div>
    </div>
  </template>
</template>
