<script lang="ts" setup>
import { ref, defineProps, watch } from 'vue'
import { type IntroductionData } from '@/components/introduction/Introduction.vue'
import Introduction from '@/components/introduction/Introduction.vue'
import {
  GameStateType,
  MiniGameIntroductionPayload,
  type MiniGamePayloadType
} from '@/flatbuffers/messageClass'
import type {
  TeambasedPongHostData,
  TeambasedPongRoundResult,
  TeambasedPongResult
} from './TeambasedPongModels'
import {
  parseTeambasedPongHostPayload,
  parseTeambasedPongRoundResult,
  parseTeambasedPongResult
} from './TeambasedPongProcessor'
import { PongRoundWinner } from './TeambasedPongModels'
import { Application } from 'vue3-pixi'
import { Graphics, Text, TextStyle } from 'pixi.js'

const props = defineProps<{
  width: number
  height: number
}>()

enum ViewState {
  None,
  Introduction,
  MiniGame,
  RoundResult,
  Results
}

const viewState = ref<ViewState>(ViewState.None)

// introduction
const intro = ref<IntroductionData>({
  title: '',
  description: '',
  time_left: 0
})

// host payload data
const payloadData = ref<TeambasedPongHostData>({
  current_round: 0,
  time_left: 0,
  map_width: 800,
  map_height: 600,
  ball_x: 0,
  ball_y: 0,
  paddle_a_x: 0,
  paddle_a_y: 0,
  paddle_b_x: 0,
  paddle_b_y: 0,
  paddle_width: 20,
  paddle_height: 120,
  team_a_players: [],
  team_b_players: []
})

// round result data
const roundResult = ref<TeambasedPongRoundResult>({
  round_winner: PongRoundWinner.TIE,
  winning_player_name: '',
  time_left: 0,
  has_next_round: false,
  next_team_a_players: [],
  next_team_b_players: []
})

// final results
const results = ref<TeambasedPongResult>({
  results: []
})

const update = (data: MiniGamePayloadType) => {
  switch (data.gamestatetype()) {
    case GameStateType.TeambasedPongHost: {
      viewState.value = ViewState.MiniGame
      payloadData.value = parseTeambasedPongHostPayload(data)
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
    case GameStateType.TeambasedPongRoundResult: {
      viewState.value = ViewState.RoundResult
      roundResult.value = parseTeambasedPongRoundResult(data)
      break
    }
    case GameStateType.TeambasedPongResult: {
      viewState.value = ViewState.Results
      results.value = parseTeambasedPongResult(data)
      break
    }
  }
  return []
}

// Render the pong game
const renderPongGame = (graphics: Graphics) => {
  graphics.clear()

  const canvasWidth = 780
  const canvasHeight = 530
  const scaleX = canvasWidth / payloadData.value.map_width
  const scaleY = canvasHeight / payloadData.value.map_height

  // Draw center line
  graphics.lineStyle(3, 0xffffff, 0.5)
  for (let y = 0; y < canvasHeight; y += 30) {
    graphics.moveTo(canvasWidth / 2, y)
    graphics.lineTo(canvasWidth / 2, y + 15)
  }

  // Draw paddles
  graphics.lineStyle(0)
  graphics.beginFill(0x00ff00) // Team A - Green

  const paddleAX = (payloadData.value.paddle_a_x + payloadData.value.map_width / 2) * scaleX
  const paddleAY = (payloadData.value.paddle_a_y + payloadData.value.map_height / 2) * scaleY
  graphics.drawRect(
    paddleAX - (payloadData.value.paddle_width / 2) * scaleX,
    paddleAY - (payloadData.value.paddle_height / 2) * scaleY,
    payloadData.value.paddle_width * scaleX,
    payloadData.value.paddle_height * scaleY
  )
  graphics.endFill()

  graphics.beginFill(0xff0000) // Team B - Red
  const paddleBX = (payloadData.value.paddle_b_x + payloadData.value.map_width / 2) * scaleX
  const paddleBY = (payloadData.value.paddle_b_y + payloadData.value.map_height / 2) * scaleY
  graphics.drawRect(
    paddleBX - (payloadData.value.paddle_width / 2) * scaleX,
    paddleBY - (payloadData.value.paddle_height / 2) * scaleY,
    payloadData.value.paddle_width * scaleX,
    payloadData.value.paddle_height * scaleY
  )
  graphics.endFill()

  // Draw ball
  graphics.beginFill(0xffffff)
  const ballX = (payloadData.value.ball_x + payloadData.value.map_width / 2) * scaleX
  const ballY = (payloadData.value.ball_y + payloadData.value.map_height / 2) * scaleY
  graphics.drawCircle(ballX, ballY, 10)
  graphics.endFill()
}

watch(
  () => payloadData.value,
  () => {
    // Trigger re-render when data changes
  },
  { deep: true }
)

const getRoundWinnerText = () => {
  switch (roundResult.value.round_winner) {
    case PongRoundWinner.TEAM_A_WON:
      return 'Team A (Green) Wins!'
    case PongRoundWinner.TEAM_B_WON:
      return 'Team B (Red) Wins!'
    case PongRoundWinner.TIE:
      return 'Tie! Both teams eliminated!'
    case PongRoundWinner.SINGLE_WINNER:
      return `${roundResult.value.winning_player_name} Wins!`
    default:
      return ''
  }
}

const formatTime = (ms: number) => {
  const seconds = Math.ceil(ms / 1000)
  return seconds.toString()
}

defineExpose({
  update
})
</script>

<template>
  <template v-if="viewState == ViewState.Introduction">
    <Introduction logoSVG="/assets/logo.svg" :data="intro" />
  </template>
  
  <template v-else-if="viewState == ViewState.MiniGame">
    <div class="flex flex-col items-center justify-center w-full h-full p-4">
      <!-- Round and Time Info -->
      <div class="flex justify-between w-full max-w-4xl mb-4">
        <div class="text-3xl text-white">Round {{ payloadData.current_round }}</div>
        <div class="text-3xl text-white">Time: {{ formatTime(payloadData.time_left) }}s</div>
      </div>

      <!-- Team Information -->
      <div class="flex justify-between w-full max-w-4xl mb-4">
        <div class="flex-1 bg-green-700 bg-opacity-50 rounded-lg p-3 mr-2">
          <div class="text-2xl text-white font-bold mb-2">Team A (Green)</div>
          <div class="text-white">
            <div v-for="player in payloadData.team_a_players" :key="player.name" class="text-lg">
              {{ player.name }}
            </div>
          </div>
        </div>
        <div class="flex-1 bg-red-700 bg-opacity-50 rounded-lg p-3 ml-2">
          <div class="text-2xl text-white font-bold mb-2">Team B (Red)</div>
          <div class="text-white">
            <div v-for="player in payloadData.team_b_players" :key="player.name" class="text-lg">
              {{ player.name }}
            </div>
          </div>
        </div>
      </div>

      <!-- Game Canvas -->
      <div class="bg-black rounded-lg overflow-hidden">
        <Application :width="780" :height="530" :backgroundAlpha="1" :backgroundColor="0x000000">
          <Graphics @render="renderPongGame" />
        </Application>
      </div>
    </div>
  </template>

  <template v-else-if="viewState == ViewState.RoundResult">
    <div class="flex flex-col items-center justify-center w-full h-full p-8">
      <div class="text-6xl text-white font-bold mb-8">{{ getRoundWinnerText() }}</div>

      <template v-if="roundResult.has_next_round">
        <div class="text-4xl text-white mb-8">Next Round Teams:</div>
        <div class="flex justify-between w-full max-w-4xl">
          <div class="flex-1 bg-green-700 bg-opacity-50 rounded-lg p-4 mr-4">
            <div class="text-3xl text-white font-bold mb-3">Team A (Green)</div>
            <div class="text-white">
              <div
                v-for="player in roundResult.next_team_a_players"
                :key="player.name"
                class="text-2xl mb-1"
              >
                {{ player.name }}
              </div>
            </div>
          </div>
          <div class="flex-1 bg-red-700 bg-opacity-50 rounded-lg p-4 ml-4">
            <div class="text-3xl text-white font-bold mb-3">Team B (Red)</div>
            <div class="text-white">
              <div
                v-for="player in roundResult.next_team_b_players"
                :key="player.name"
                class="text-2xl mb-1"
              >
                {{ player.name }}
              </div>
            </div>
          </div>
        </div>
      </template>
    </div>
  </template>

  <template v-else-if="viewState == ViewState.Results">
    <div class="flex flex-col items-center justify-center w-full h-full p-8">
      <div class="text-6xl text-white font-bold mb-8">Final Results</div>
      <div class="w-full max-w-2xl">
        <div
          v-for="result in results.results"
          :key="result.name"
          class="flex justify-between items-center bg-gray-800 bg-opacity-70 rounded-lg p-4 mb-3"
        >
          <div class="flex items-center gap-4">
            <div class="text-4xl text-white font-bold w-16 text-center">
              {{ result.placement }}
            </div>
            <div class="text-3xl text-white">{{ result.name }}</div>
          </div>
          <div class="text-2xl text-white">{{ result.rounds_won }} rounds won</div>
        </div>
      </div>
    </div>
  </template>
</template>

<style scoped></style>
