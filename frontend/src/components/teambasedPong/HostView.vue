<script lang="ts" setup>
import { ref, shallowRef } from 'vue'
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
import TeambasedPongPrepView from './TeambasedPongPrepView.vue'
import TeambasedPongGameView from './TeambasedPongGameView.vue'

defineProps<{
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
const gameViewRef = ref<InstanceType<typeof TeambasedPongGameView>>()

// introduction
const intro = ref<IntroductionData>({
  title: '',
  description: '',
  time_left: 0
})

// host payload data
const payloadData = shallowRef<TeambasedPongHostData>({
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
  update,
  ...(import.meta.env.DEV
    ? {
        // Mirrors update()'s real dispatch (same synchronous viewState/data assignment) — for
        // driving the RoundPrep <-> MiniGame transition (the two-component split) and the
        // real prop-passing path from a dev harness without building fake FlatBuffers.
        __devUpdateRoundPrep: (data: TeambasedPongRoundPrepData) => {
          viewState.value = ViewState.RoundPrep
          roundPrepData.value = data
        },
        __devUpdateHost: (data: TeambasedPongHostData) => {
          viewState.value = ViewState.MiniGame
          payloadData.value = data
        },
        __devSmoothedDirection: (team: 'a' | 'b', name: string) =>
          gameViewRef.value?.__devSmoothedDirection?.(team, name)
      }
    : {})
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
        <TeambasedPongPrepView />
      </div>

      <!-- Foreground: round info + team rosters -->
      <div class="relative z-10 flex flex-col items-center justify-center w-full h-full p-8">
        <div class="text-5xl text-white font-bold mb-8">
          Round {{ roundPrepData.current_round }}
        </div>
        <div class="text-3xl text-white mb-8">
          Starting in {{ formatTime(roundPrepData.time_left) }}s
        </div>

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
              <div
                v-for="player in payloadData.team_a_players"
                :key="player.name"
                class="text-sm mb-0.5"
              >
                {{ player.name }}
              </div>
            </div>
          </div>

          <!-- Game Canvas — aspect-[4/3] matches the 800x600 world so this box's own height is
               driven by its width (row height with it, not the other way around), giving
               useGameCanvas's letterboxing a properly-proportioned box to fill edge-to-edge
               instead of shrinking to whatever short height an all-text flex row would otherwise
               settle on. -->
          <div class="bg-black rounded-lg overflow-hidden flex-1 aspect-[4/3]">
            <TeambasedPongGameView ref="gameViewRef" :host-data="payloadData" />
          </div>

          <!-- Team B (right side) -->
          <div class="w-40 bg-red-700 bg-opacity-50 rounded-lg p-3 ml-2 flex flex-col">
            <div class="text-lg text-white font-bold mb-2">Team B</div>
            <div class="text-white overflow-y-auto flex-1">
              <div
                v-for="player in payloadData.team_b_players"
                :key="player.name"
                class="text-sm mb-0.5"
              >
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
