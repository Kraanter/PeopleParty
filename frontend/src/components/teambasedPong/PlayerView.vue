<script lang="ts" setup>
import { ref, computed } from 'vue'
import TimeComponent from '../TimeComponent.vue'
import { type IntroductionData } from '@/components/introduction/Introduction.vue'
import {
  GameStateType,
  MiniGameIntroductionPayload,
  type MiniGamePayloadType
} from '@/flatbuffers/messageClass'
import { useWebSocketStore } from '@/stores/confettiStore'
import type {
  TeambasedPongPlayerData,
  TeambasedPongRoundResult,
  TeambasedPongResult,
  TeambasedPongRoundPrepData
} from './TeambasedPongModels'
import {
  parseTeambasedPongPlayerPayload,
  parseTeambasedPongRoundResult,
  parseTeambasedPongResult,
  parseTeambasedPongRoundPrepPayload
} from './TeambasedPongProcessor'
import { PongTeam, PongRoundWinner } from './TeambasedPongModels'
import JoystickComponent from '../shared/JoystickComponent.vue'
import { sendPlayerAction } from '@/util/joystickMessageBuilder'

const websocketStore = useWebSocketStore()

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

// player payload data
const payloadData = ref<TeambasedPongPlayerData>({
  current_round: 0,
  time_left: 0,
  your_team: PongTeam.SPECTATOR,
  is_still_playing: true
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

const personalResult = computed(() => {
  return results.value.results.find((result) => result.name === websocketStore.clientName)
})

const update = (data: MiniGamePayloadType) => {
  switch (data.gamestatetype()) {
    case GameStateType.TeambasedPongPlayer: {
      viewState.value = ViewState.MiniGame
      payloadData.value = parseTeambasedPongPlayerPayload(data)
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

const move = ({ y }: any) => {
  // Only send Y axis for pong paddle control
  sendPlayerAction('teambasedPong', 0, y)
}

const stop = () => {
  sendPlayerAction('teambasedPong', 0, 0)
}

const getTeamColor = () => {
  switch (payloadData.value.your_team) {
    case PongTeam.TEAM_A:
      return 'bg-green-700'
    case PongTeam.TEAM_B:
      return 'bg-red-700'
    default:
      return 'bg-gray-700'
  }
}

const getTeamName = () => {
  switch (payloadData.value.your_team) {
    case PongTeam.TEAM_A:
      return 'Team A (Green)'
    case PongTeam.TEAM_B:
      return 'Team B (Red)'
    default:
      return 'Spectator'
  }
}

const getRoundPrepTeamColor = () => {
  const playerName = websocketStore.clientName
  const inTeamA = roundPrepData.value.team_a_players.some((p) => p.name === playerName)
  if (inTeamA) return 'bg-green-700'
  const inTeamB = roundPrepData.value.team_b_players.some((p) => p.name === playerName)
  if (inTeamB) return 'bg-red-700'
  return 'bg-gray-700'
}

const getRoundPrepTeamName = () => {
  const playerName = websocketStore.clientName
  const inTeamA = roundPrepData.value.team_a_players.some((p) => p.name === playerName)
  if (inTeamA) return 'Team A (Green)'
  const inTeamB = roundPrepData.value.team_b_players.some((p) => p.name === playerName)
  if (inTeamB) return 'Team B (Red)'
  return 'Eliminated'
}

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

const getNextTeamName = () => {
  const playerName = websocketStore.clientName
  const inTeamA = roundResult.value.next_team_a_players.some((p) => p.name === playerName)
  const inTeamB = roundResult.value.next_team_b_players.some((p) => p.name === playerName)

  if (inTeamA) return 'Team A (Green)'
  if (inTeamB) return 'Team B (Red)'
  return 'Not in next round'
}

const isInNextRound = () => {
  const playerName = websocketStore.clientName
  return (
    roundResult.value.next_team_a_players.some((p) => p.name === playerName) ||
    roundResult.value.next_team_b_players.some((p) => p.name === playerName)
  )
}

const formatTime = (ms: number) => {
  const seconds = Math.ceil(ms / 1000)
  return seconds.toString()
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

  <template v-else-if="viewState == ViewState.RoundPrep">
    <div class="flex flex-col items-center justify-center w-full h-full p-6">
      <div class="text-4xl text-white font-bold mb-6">Round {{ roundPrepData.current_round }}</div>
      <div class="text-2xl text-white mb-6">Starting in {{ formatTime(roundPrepData.time_left) }}s</div>

      <div
        :class="[getRoundPrepTeamColor(), 'p-6 rounded-lg mb-4']"
      >
        <div class="text-3xl text-white font-bold text-center">{{ getRoundPrepTeamName() }}</div>
      </div>
    </div>
  </template>

  <template v-else-if="viewState == ViewState.MiniGame">
    <div class="flex flex-col h-full w-full justify-center items-center p-4">
      <!-- Round and Team Info -->
      <div class="text-3xl text-white mb-4">Round {{ payloadData.current_round }}</div>
      <div class="text-2xl text-white mb-2">Time: {{ formatTime(payloadData.time_left) }}s</div>

      <div :class="[getTeamColor(), 'p-6 rounded-lg mb-6']">
        <div class="text-4xl text-white font-bold text-center">{{ getTeamName() }}</div>
      </div>

      <template v-if="!payloadData.is_still_playing">
        <div class="text-3xl text-red-500 font-bold mb-4">You've been eliminated!</div>
      </template>

      <!-- Joystick -->
      <div class="flex-1 flex items-center justify-center">
        <JoystickComponent
          :disabled="!payloadData.is_still_playing"
          class="no-project-style"
          :size="200"
          base-color="lightgray"
          stick-color="black"
          :throttle="100"
          @move="move"
          @stop="stop"
        />
      </div>

      <div class="text-white text-center text-lg mb-4">
        {{ payloadData.is_still_playing ? 'Move the joystick UP/DOWN to control your paddle!' : 'Watch the game on the big screen!' }}
      </div>
    </div>
  </template>

  <template v-else-if="viewState == ViewState.RoundResult">
    <div class="flex flex-col items-center justify-center w-full h-full p-8">
      <div class="text-5xl text-white font-bold mb-8 text-center">
        {{ getRoundWinnerText() }}
      </div>

      <template v-if="roundResult.has_next_round">
        <div class="text-3xl text-white mb-6">Next Round:</div>
        <div
          v-if="isInNextRound()"
          class="text-4xl text-white font-bold p-6 rounded-lg"
          :class="[
            getNextTeamName() === 'Team A (Green)' ? 'bg-green-700' : 'bg-red-700'
          ]"
        >
          You're in {{ getNextTeamName() }}!
        </div>
        <div v-else class="text-3xl text-red-500 font-bold">
          You've been eliminated!
        </div>
      </template>
      <template v-else>
        <div class="text-3xl text-white">Game Over!</div>
      </template>
    </div>
  </template>

  <template v-else-if="viewState == ViewState.Results">
    <div
      v-if="personalResult"
      class="flex flex-col gap-4 w-full h-full justify-center items-center p-4"
    >
      <div class="text-5xl text-white font-bold mb-4">You placed:</div>
      <div class="text-8xl text-white font-bold mb-6">{{ formatOrdinals(personalResult.placement) }}</div>

      <div class="flex flex-col justify-center items-center bg-gray-800 bg-opacity-70 rounded-lg p-6">
        <div class="text-2xl text-white mb-2">Rounds Won:</div>
        <div class="text-6xl text-white font-bold">{{ personalResult.rounds_won }}</div>
      </div>
    </div>
  </template>
</template>

<style scoped></style>
