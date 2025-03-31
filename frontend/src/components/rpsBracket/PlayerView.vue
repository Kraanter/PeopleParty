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
  None = 0,
  Rock = 1,
  Paper = 2,
  Scissors = 3
}

const RPSChoiceMap = {
  [RPSChoice.None]: 'Surrender',
  [RPSChoice.Rock]: 'Rock',
  [RPSChoice.Paper]: 'Paper',
  [RPSChoice.Scissors]: 'Scissors'
}

const RPSMap = {
  [RPSChoice.Rock]: 'Rock',
  [RPSChoice.Paper]: 'Paper',
  [RPSChoice.Scissors]: 'Scissors'
}

interface playerDataPayload {
  choice: RPSChoice
  opponentName: string
  opponentChoice: RPSChoice
  winner: string
  time_left: number
}

// game data
const playerData = ref<playerDataPayload>()

const timerId = ref(null);
const showMatchResultBool = ref(false);
const firstTime = ref(true);

function startTimer() {
  if (timerId.value) {
    clearTimeout(timerId.value);
  }

  showMatchResultBool.value = true;

  timerId.value = setTimeout(() => {
    showMatchResultBool.value = false;
  }, 6000);
};

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

      if (firstTime.value) {
        firstTime.value = false
      } else {
        startTimer() // to ensure the match result is shown after match
      }

      playerData.value = {
        choice: (input.choice() || 0) as RPSChoice,
        opponentChoice: (input.opponentChoice() || 0) as RPSChoice,
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

function isResultScreen() {
  return playerData.value.opponentChoice && playerData.value.winner != '' || playerData.value.choice != RPSChoice.None && playerData.value.choice == playerData.value.opponentChoice
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
  <template v-else-if="viewState === ViewState.MiniGame && playerData">
    <div class="flex flex-col m-2 text-center gap-4 h-full">
      <div v-if="!showMatchResultBool && playerData.winner != '' && playerData.winner != playerData.opponentName">
        <span class="text text-white text-2xl mr-4">Waiting for opponent...</span>
      </div>
      <div v-else-if="!showMatchResultBool && playerData.winner != ''">
        <span class="text text-white text-2xl mr-4">You are eliminated<br> from this tournement.</span>
      </div>
      <div v-else class="w-full">
        <!-- top component that shows opponents name //both screens -->
        <div class="w-full mt-4">
          <div>
            <span class="text-2xl text-white">Opponent:</span>
          </div>
          <div>
            <span class="text-5xl text-primary">{{ playerData.opponentName }}</span>
          </div>
        </div>
        <!-- show if you won or lost //only on result screen -->
        <div v-if="isResultScreen() && showMatchResultBool" class="w-full mt-20">
          <div v-if="playerData.winner != '' && playerData.winner != playerData.opponentName">
            <span class="text-6xl text-primary">You Won!</span>
          </div>
          <div v-else-if="playerData.winner != ''">
            <span class="text-6xl text-primary">You Lost!</span>
          </div>
        </div>
        <!-- card component that shows the outcome of the match //only on result screen -->
        <div v-if="isResultScreen()" class="mt-16 top-2 right-1 left-1">
          <n-card 
            class="text-2xl text-white w-full" 
            :class="{ 
              'background-green' : playerData.winner != '' && playerData.winner != playerData.opponentName, 
              'background-red' : playerData.winner != '' && playerData.winner == playerData.opponentName
            }"
          >
            <div class="flex justify-center items-center gap-10">
              <div>
                <p class="text-xl mb-4">You:</p>
                <div v-if="!Number.isNaN(playerData.choice)">
                  <img
                  class="m-auto size-32 aspect-square"
                  :src="`/assets/games/rpsBracket/${RPSChoiceMap[playerData.choice]?.toLowerCase()}.svg`"
                  />
                  {{ RPSChoiceMap[playerData.choice] }}
                </div>
              </div>
              <div v-if="playerData.opponentChoice && playerData.winner != '' || playerData.choice != RPSChoice.None && playerData.choice == playerData.opponentChoice">
                <p class="text-xl mb-4">{{ playerData.opponentName }}:</p>
                <div v-if="!Number.isNaN(playerData.opponentChoice)">
                  <img
                  class="m-auto size-32 aspect-square"
                  :src="`/assets/games/rpsBracket/${RPSChoiceMap[playerData.opponentChoice]?.toLowerCase()}.svg`"
                  />
                  {{ RPSChoiceMap[playerData.opponentChoice] }}
                </div>
              </div>
            </div>
          </n-card>
        </div>
        <!-- time component //only on match screen-->
        <div v-if="!isResultScreen()" class="mt-16 w-full">
          <div class="flex justify-center items-center">
            <div v-if="playerData.time_left > 0">
              <TimeComponent :timeLeft="playerData.time_left" />
            </div>
          </div>
        </div>
        <!-- choice buttons //only on match screen -->
        <div v-if="!isResultScreen()" class="w-full mt-36 px-4">
          <div class="grid grid-cols-3 gap-4">
            <div v-for="(choice, i) in RPSMap" @click="player_action(i)" :key="choice">
              <PartyButton
                :disabled="playerData.winner != ''"
                class="m-2 !text-lg"
                :class="{
                  '!bg-sky-400': i == playerData.choice,
                  'bg-primary': i != playerData.choice
                }"
                @click="player_action(i)"
              >
                <img
                  @click="player_action(i)"
                  class="size-12 m-auto"
                  :src="`/assets/games/rpsBracket/${choice.toLowerCase()}.svg`"
                />
                {{ choice }}
              </PartyButton>
            </div>
          </div>
        </div>
        <!-- outcome text (eliminated or waiting on next match to start) //only on result screen -->
        <div class="w-full flex mt-4 px-4 justify-center items-center">
          <div v-if="playerData.winner != '' && playerData.winner != playerData.opponentName">
            <span class="text text-white text-3xl mr-4">
              <div> Waiting for next </div>
              <div> match to start! </div>
            </span>
          </div>
          <div v-else-if="playerData.winner != ''">
            <span class="text text-white text-3xl mr-4">
              <div> You are eliminated </div>
              <div> from this tournement. </div>
            </span>
          </div>
        </div>
      </div>
    </div>
  </template>
</template>
<style scoped>
.background-green {
  background-color: rgb(0 255 0 / .5);
}

.background-red {
  background-color: rgb(255 0 0 / .5);
}
</style>