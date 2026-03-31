<script lang="ts" setup>
import { ref, computed } from 'vue'
import * as flatbuffers from 'flatbuffers'
import TimeComponent from '../TimeComponent.vue'
import { type IntroductionData } from '@/components/introduction/Introduction.vue'
import {
  GameStatePayload,
  GameStateType,
  MessageType,
  MiniGameIntroductionPayload,
  MiniGamePayloadType,
  Payload,
  ShellGamePlayerInputPayload
} from '@/flatbuffers/messageClass'
import { buildMessage } from '@/util/flatbufferMessageBuilder'
import { useWebSocketStore } from '@/stores/confettiStore'
import { parseShellGamePlayerPayload, parseShellGameResult } from './ShellGameProcessor'
import type { ShellGamePlayerData, ShellGameResult } from './ShellGameModels'
import { ShellGamePhase } from './ShellGameModels'

defineProps<{
  width: number
  height: number
}>()

const websocketStore = useWebSocketStore()

// ─── State ───────────────────────────────────────────────────────────────────

enum ViewState {
  None,
  Introduction,
  MiniGame,
  Results
}

const viewState = ref<ViewState>(ViewState.None)
const intro = ref<IntroductionData>({ title: '', description: '', time_left: 0 })

const playerData = ref<ShellGamePlayerData>({
  phase: ShellGamePhase.REVEAL,
  current_round: 0,
  time_left: 0,
  num_cups: 3,
  is_eliminated: false,
  has_guessed: false,
  was_correct: false
})

const gameResult = ref<ShellGameResult | null>(null)

// ─── Computed ─────────────────────────────────────────────────────────────────

const personalResult = computed(() =>
  gameResult.value?.results.find((r) => r.name === websocketStore.clientName)
)

const cupButtons = computed(() => {
  return Array.from({ length: playerData.value.num_cups }, (_, i) => i)
})

// ─── Ordinal formatting ───────────────────────────────────────────────────────

const pr = new Intl.PluralRules('en-US', { type: 'ordinal' })
const suffixes = new Map([['one', 'st'], ['two', 'nd'], ['few', 'rd'], ['other', 'th']])
const formatOrdinals = (n: number) => `${n}${suffixes.get(pr.select(n))}`

// ─── Input sending ────────────────────────────────────────────────────────────

const sendCupGuess = (cupIndex: number) => {
  if (playerData.value.has_guessed || playerData.value.is_eliminated) return
  if (playerData.value.phase !== ShellGamePhase.GUESS) return

  const builder = new flatbuffers.Builder()

  const inputPayload = ShellGamePlayerInputPayload.createShellGamePlayerInputPayload(
    builder,
    cupIndex
  )

  const gameName = builder.createString('shellGame')

  const mgPayload = MiniGamePayloadType.createMiniGamePayloadType(
    builder,
    gameName,
    GameStateType.ShellGamePlayerInput,
    GameStatePayload.ShellGamePlayerInputPayload,
    inputPayload
  )

  websocketStore.sendMessage(
    buildMessage(builder, mgPayload, MessageType.MiniGame, Payload.MiniGamePayloadType)
  )
}

// ─── Update handler ───────────────────────────────────────────────────────────

const update = (data: MiniGamePayloadType) => {
  switch (data.gamestatetype()) {
    case GameStateType.ShellGamePlayer: {
      viewState.value = ViewState.MiniGame
      playerData.value = parseShellGamePlayerPayload(data)
      break
    }
    case GameStateType.ShellGameResult: {
      viewState.value = ViewState.Results
      gameResult.value = parseShellGameResult(data)
      break
    }
    case GameStateType.MiniGameIntroduction: {
      viewState.value = ViewState.Introduction
      const p: MiniGameIntroductionPayload = data.gamestatepayload(new MiniGameIntroductionPayload())
      intro.value = {
        title: p.name() || '',
        description: p.instruction() || '',
        time_left: Number(p.timeLeft())
      }
      break
    }
  }
  return []
}

defineExpose({ update })
</script>

<template>
  <!-- Introduction -->
  <template v-if="viewState === ViewState.Introduction">
    <div class="flex flex-col h-full justify-center items-center gap-6 p-6 text-center">
      <TimeComponent :timeLeft="intro.time_left" />
      <p class="text-3xl text-white font-bold">{{ intro.title }}</p>
      <p class="text-xl text-gray-200">{{ intro.description }}</p>
    </div>
  </template>

  <!-- In-game: eliminated -->
  <template v-else-if="viewState === ViewState.MiniGame && playerData.is_eliminated">
    <div class="flex flex-col h-full justify-center items-center gap-4 p-6 text-center">
      <div class="text-6xl">😬</div>
      <p class="text-4xl text-red-400 font-bold">Eliminated!</p>
      <p class="text-xl text-gray-300">You survived {{ playerData.current_round - 1 }} round{{ playerData.current_round !== 2 ? 's' : '' }}.</p>
      <p class="text-lg text-gray-400 mt-4">Watch the others!</p>
    </div>
  </template>

  <!-- In-game: REVEAL phase -->
  <template v-else-if="viewState === ViewState.MiniGame && playerData.phase === ShellGamePhase.REVEAL">
    <div class="flex flex-col h-full justify-center items-center gap-6 p-6 text-center">
      <div class="text-7xl">👀</div>
      <p class="text-4xl text-white font-bold">Watch the ball!</p>
      <p class="text-2xl text-yellow-300">Round {{ playerData.current_round }}</p>
      <div class="mt-4">
        <TimeComponent :timeLeft="playerData.time_left" />
      </div>
    </div>
  </template>

  <!-- In-game: SHUFFLE phase -->
  <template v-else-if="viewState === ViewState.MiniGame && playerData.phase === ShellGamePhase.SHUFFLE">
    <div class="flex flex-col h-full justify-center items-center gap-6 p-6 text-center">
      <div class="text-7xl">🔄</div>
      <p class="text-4xl text-white font-bold">Don't lose it!</p>
      <p class="text-2xl text-yellow-300">Track the cup with the ball…</p>
    </div>
  </template>

  <!-- In-game: GUESS phase -->
  <template v-else-if="viewState === ViewState.MiniGame && playerData.phase === ShellGamePhase.GUESS">
    <div class="flex flex-col h-full justify-center items-center gap-4 p-4 text-center">
      <p class="text-3xl text-white font-bold">Which cup?</p>
      <div class="mt-2">
        <TimeComponent :timeLeft="playerData.time_left" />
      </div>

      <!-- Cup buttons -->
      <div v-if="!playerData.has_guessed" class="flex flex-wrap justify-center gap-4 mt-4">
        <button
          v-for="i in cupButtons"
          :key="i"
          class="w-24 h-24 rounded-2xl text-3xl font-bold text-white shadow-lg active:scale-95 transition-transform"
          style="background-color: #8b5e3c"
          @click="sendCupGuess(i)"
        >
          🪣 {{ i + 1 }}
        </button>
      </div>

      <!-- Already guessed -->
      <div v-else class="flex flex-col items-center gap-3 mt-4">
        <div class="text-5xl">✅</div>
        <p class="text-2xl text-green-400 font-bold">Guess locked in!</p>
        <p class="text-lg text-gray-300">Waiting for others…</p>
      </div>
    </div>
  </template>

  <!-- In-game: ROUND_RESULT phase -->
  <template v-else-if="viewState === ViewState.MiniGame && playerData.phase === ShellGamePhase.ROUND_RESULT">
    <div class="flex flex-col h-full justify-center items-center gap-6 p-6 text-center">
      <template v-if="playerData.was_correct">
        <div class="text-7xl">🎉</div>
        <p class="text-5xl text-green-400 font-bold">Correct!</p>
        <p class="text-2xl text-white">You survived round {{ playerData.current_round }}!</p>
      </template>
      <template v-else>
        <div class="text-7xl">😭</div>
        <p class="text-5xl text-red-400 font-bold">Wrong!</p>
        <p class="text-2xl text-white">Eliminated in round {{ playerData.current_round }}</p>
      </template>
    </div>
  </template>

  <!-- Final results -->
  <template v-else-if="viewState === ViewState.Results">
    <div class="flex flex-col h-full justify-center items-center gap-4 p-6 text-center">
      <template v-if="personalResult">
        <p class="text-3xl text-white font-bold">You placed:</p>
        <p class="text-8xl text-yellow-300 font-bold">{{ formatOrdinals(personalResult.placement) }}</p>
        <p class="text-xl text-gray-300">
          Survived {{ personalResult.rounds_survived }} round{{ personalResult.rounds_survived !== 1 ? 's' : '' }}
        </p>
      </template>
      <template v-else>
        <p class="text-3xl text-white font-bold">Game Over!</p>
      </template>
    </div>
  </template>
</template>
