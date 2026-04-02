<script lang="ts" setup>
import { ref, watch, computed, onUnmounted } from 'vue'
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
  TeambasedPongResult,
  TeambasedPongRoundPrepData
} from './TeambasedPongModels'
import {
  parseTeambasedPongHostPayload,
  parseTeambasedPongRoundResult,
  parseTeambasedPongResult,
  parseTeambasedPongRoundPrepPayload
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
  RoundPrep,
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

// round prep data
const roundPrepData = ref<TeambasedPongRoundPrepData>({
  current_round: 0,
  time_left: 0,
  team_a_players: [],
  team_b_players: []
})

// final results
const results = ref<TeambasedPongResult>({
  results: []
})

// Canvas sizing - dynamically tracks container width and maintains aspect ratio
const gameCanvasRef = ref<HTMLElement | null>(null)
const canvasWidth = ref(780)
const MAP_ASPECT = 800 / 600
const canvasHeight = computed(() => Math.round(canvasWidth.value / MAP_ASPECT))
const scale = computed(() => canvasWidth.value / payloadData.value.map_width)

// Per-player direction smoothing state to avoid marker teleporting between network ticks.
const smoothedTeamAInputs = new Map<string, number>()
const smoothedTeamBInputs = new Map<string, number>()
const inputSmoothingFactor = 0.2

const clampDirection = (value: number) => Math.max(0, Math.min(100, value))

const getSmoothedDirection = (
  smoothingMap: Map<string, number>,
  playerKey: string,
  targetDirection: number
) => {
  const target = clampDirection(targetDirection)
  const current = smoothingMap.get(playerKey) ?? target
  const next = current + (target - current) * inputSmoothingFactor
  smoothingMap.set(playerKey, next)
  return next
}

const pruneMissingPlayers = (smoothingMap: Map<string, number>, activePlayerKeys: Set<string>) => {
  for (const key of smoothingMap.keys()) {
    if (!activePlayerKeys.has(key)) {
      smoothingMap.delete(key)
    }
  }
}

let resizeObserver: ResizeObserver | null = null
watch(gameCanvasRef, (el) => {
  resizeObserver?.disconnect()
  if (el) {
    canvasWidth.value = el.clientWidth
    resizeObserver = new ResizeObserver((entries) => {
      canvasWidth.value = entries[0].contentRect.width
    })
    resizeObserver.observe(el)
  }
})
onUnmounted(() => {
  resizeObserver?.disconnect()
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
    case GameStateType.TeambasedPongRoundPrep: {
      viewState.value = ViewState.RoundPrep
      roundPrepData.value = parseTeambasedPongRoundPrepPayload(data)
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

// Render static pong field for round prep background
const renderPrepBackground = (graphics: Graphics) => {
  graphics.clear()

  const cw = canvasWidth.value
  const ch = canvasHeight.value
  const s = cw / 800 // scale from map coords to canvas

  // Draw center line
  graphics.lineStyle(3, 0xffffff, 0.5)
  for (let y = 0; y < ch; y += 30) {
    graphics.moveTo(cw / 2, y)
    graphics.lineTo(cw / 2, y + 15)
  }

  // Draw paddles at center positions
  graphics.lineStyle(0)
  graphics.beginFill(0x00ff00)
  graphics.drawRect(
    30 * s - (20 / 2) * s,
    ch / 2 - (120 / 2) * s,
    20 * s,
    120 * s
  )
  graphics.endFill()

  graphics.beginFill(0xff0000)
  graphics.drawRect(
    (800 - 30) * s - (20 / 2) * s,
    ch / 2 - (120 / 2) * s,
    20 * s,
    120 * s
  )
  graphics.endFill()

  // Draw ball at center
  graphics.beginFill(0xffffff)
  graphics.drawCircle(cw / 2, ch / 2, 10 * s)
  graphics.endFill()
}

// Render the pong game
const renderPongGame = (graphics: Graphics) => {
  graphics.clear()

  const cw = canvasWidth.value
  const ch = canvasHeight.value
  const s = scale.value
  const mw = payloadData.value.map_width
  const mh = payloadData.value.map_height
  const halfPaddleHeight = (payloadData.value.paddle_height / 2) * s

  pruneMissingPlayers(
    smoothedTeamAInputs,
    new Set(payloadData.value.team_a_players.map((player) => player.name))
  )
  pruneMissingPlayers(
    smoothedTeamBInputs,
    new Set(payloadData.value.team_b_players.map((player) => player.name))
  )

  // Draw center line
  graphics.lineStyle(3, 0xffffff, 0.5)
  for (let y = 0; y < ch; y += 30) {
    graphics.moveTo(cw / 2, y)
    graphics.lineTo(cw / 2, y + 15)
  }

  // Draw paddles
  graphics.lineStyle(0)
  graphics.beginFill(0x00ff00) // Team A - Green
  const paddleAX = (payloadData.value.paddle_a_x + mw / 2) * s
  const paddleAY = (payloadData.value.paddle_a_y + mh / 2) * s
  graphics.drawRect(
    paddleAX - (payloadData.value.paddle_width / 2) * s,
    paddleAY - (payloadData.value.paddle_height / 2) * s,
    payloadData.value.paddle_width * s,
    payloadData.value.paddle_height * s
  )
  graphics.endFill()
  // Draw each players input
  for (const player of payloadData.value.team_a_players) {
    const smoothedDirection = getSmoothedDirection(smoothedTeamAInputs, player.name, player.direction)
    if (Math.abs(smoothedDirection - 50) < 0.5) continue // skip near-neutral input
    const inputY = paddleAY + ((smoothedDirection - 50) / 50) * halfPaddleHeight
    graphics.beginFill(0x00ff00)
    graphics.drawCircle(paddleAX - 20 * s, inputY, 8 * s)
    graphics.endFill()
  }

  graphics.beginFill(0xff0000) // Team B - Red
  const paddleBX = (payloadData.value.paddle_b_x + mw / 2) * s
  const paddleBY = (payloadData.value.paddle_b_y + mh / 2) * s
  graphics.drawRect(
    paddleBX - (payloadData.value.paddle_width / 2) * s,
    paddleBY - (payloadData.value.paddle_height / 2) * s,
    payloadData.value.paddle_width * s,
    payloadData.value.paddle_height * s
  )
  graphics.endFill()
  // Draw each players input
  for (const player of payloadData.value.team_b_players) {
    const smoothedDirection = getSmoothedDirection(smoothedTeamBInputs, player.name, player.direction)
    if (Math.abs(smoothedDirection - 50) < 0.5) continue // skip near-neutral input
    const inputY = paddleBY + ((smoothedDirection - 50) / 50) * halfPaddleHeight
    graphics.beginFill(0xff0000)
    graphics.drawCircle(paddleBX + 20 * s, inputY, 8 * s)
    graphics.endFill()
  }

  // Draw ball
  graphics.beginFill(0xffffff)
  const ballX = (payloadData.value.ball_x + mw / 2) * s
  const ballY = (payloadData.value.ball_y + mh / 2) * s
  graphics.drawCircle(ballX, ballY, 10 * s)
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

  <template v-else-if="viewState == ViewState.RoundPrep">
    <div class="m-4 relative flex items-center justify-center w-full h-full">
      <!-- Background: static pong field with blur -->
      <div class="absolute inset-0 flex items-center justify-center prep-background">
        <Application :width="canvasWidth" :height="canvasHeight" :backgroundAlpha="1" :backgroundColor="0x000000">
          <Graphics @render="renderPrepBackground" />
        </Application>
      </div>

      <!-- Foreground: round info + team rosters -->
      <div class="relative z-10 flex flex-col items-center justify-center w-full h-full p-8">
        <div class="text-5xl text-white font-bold mb-8">Round {{ roundPrepData.current_round }}</div>
        <div class="text-3xl text-white mb-8">Starting in {{ formatTime(roundPrepData.time_left) }}s</div>

        <div class="flex justify-between w-full max-w-4xl">
          <div class="flex-1 bg-green-700 bg-opacity-50 rounded-lg p-6 mr-4">
            <div class="text-3xl text-white font-bold mb-4">Team A (Green)</div>
            <div class="text-white">
              <div
                v-for="player in roundPrepData.team_a_players"
                :key="player.name"
                class="text-2xl mb-1"
              >
                {{ player.name }}
              </div>
            </div>
          </div>
          <div class="flex-1 bg-red-700 bg-opacity-50 rounded-lg p-6 ml-4">
            <div class="text-3xl text-white font-bold mb-4">Team B (Red)</div>
            <div class="text-white">
              <div
                v-for="player in roundPrepData.team_b_players"
                :key="player.name"
                class="text-2xl mb-1"
              >
                {{ player.name }}
              </div>
            </div>
          </div>
        </div>
      </div>
    </div>
  </template>
  
  <template v-else-if="viewState == ViewState.MiniGame">
    <div class="m-6">
      <div class="flex flex-col items-center justify-center w-full h-full p-4">
        <!-- Round and Time Info -->
        <div class="flex justify-between w-full max-w-7xl mb-2">
          <div class="text-3xl text-white">Round {{ payloadData.current_round }}</div>
          <div class="text-3xl text-white">Time: {{ formatTime(payloadData.time_left) }}s</div>
        </div>
        
        <!-- Game area: teams on sides, canvas in center -->
        <div class="flex items-stretch w-full max-w-7xl">
          <!-- Team A (left side) -->
          <div class="w-40 bg-green-700 bg-opacity-50 rounded-lg p-3 mr-2 flex flex-col">
            <div class="text-lg text-white font-bold mb-2">Team A</div>
            <div class="text-white overflow-y-auto flex-1">
              <div v-for="player in payloadData.team_a_players" :key="player.name" class="text-sm mb-0.5">
                {{ player.name }}
              </div>
            </div>
          </div>
          
          <!-- Game Canvas -->
          <div ref="gameCanvasRef" class="bg-black rounded-lg overflow-hidden flex-1">
            <Application :width="canvasWidth" :height="canvasHeight" :backgroundAlpha="1" :backgroundColor="0x000000">
              <Graphics @render="renderPongGame" />
            </Application>
          </div>
          
          <!-- Team B (right side) -->
          <div class="w-40 bg-red-700 bg-opacity-50 rounded-lg p-3 ml-2 flex flex-col">
            <div class="text-lg text-white font-bold mb-2">Team B</div>
            <div class="text-white overflow-y-auto flex-1">
              <div v-for="player in payloadData.team_b_players" :key="player.name" class="text-sm mb-0.5">
                {{ player.name }}
              </div>
            </div>
          </div>
        </div>
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

<style scoped>
.prep-background {
  filter: blur(4px);
  opacity: 0.7;
}
</style>
