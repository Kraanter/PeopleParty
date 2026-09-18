<script setup lang="ts">
import { ref, onMounted } from 'vue'
import ShellShuffleGameView from '@/components/shellShuffle/ShellShuffleGameView.vue'
import { ShellShufflePhase } from '@/components/shellShuffle/ShellShuffleModels'
import type {
  ShellShuffleHostData,
  ShellShuffleRoundResult
} from '@/components/shellShuffle/ShellShuffleModels'

// Phase 2 migration verification harness — mounts the real ShellShuffleGameView directly and
// drives it via its dev-only __devPush hook, with hand-written plain-object scenarios instead
// of real FlatBuffers (the decode layer isn't touched by this migration, so it isn't worth
// testing here). Dev-only (see router/index.ts), never present in a production build.

const gameViewRef = ref<InstanceType<typeof ShellShuffleGameView>>()
const roundResult = ref<ShellShuffleRoundResult | null>(null)
const containerWide = ref(true)
const canvasElementCount = ref(0)

function cupsAt(count: number) {
  const spacing = 800 / (count + 1)
  return Array.from({ length: count }, (_, i) => ({ x_pos: spacing * (i + 1), depth: 0 }))
}

function updateCanvasCount() {
  requestAnimationFrame(() => {
    canvasElementCount.value = document.querySelectorAll('#harness-mount canvas').length
  })
}

onMounted(updateCanvasCount)

function push(data: Partial<ShellShuffleHostData>) {
  gameViewRef.value?.__devPush?.({
    phase: ShellShufflePhase.REVEAL,
    current_round: 1,
    time_left: 5000,
    cups: cupsAt(3),
    ball_cup_index: -1,
    active_players: 4,
    map_width: 800,
    ...data
  })
  updateCanvasCount()
}

function showReveal(cupCount: number) {
  push({
    phase: ShellShufflePhase.REVEAL,
    cups: cupsAt(cupCount),
    ball_cup_index: 0,
    time_left: 3000
  })
}

function showShuffle(cupCount: number) {
  push({
    phase: ShellShufflePhase.SHUFFLE,
    cups: cupsAt(cupCount).map((c, i) => ({ ...c, depth: i % 2 === 0 ? 0.8 : -0.8 })),
    ball_cup_index: -1,
    time_left: -1
  })
}

function showGuess(cupCount: number) {
  push({
    phase: ShellShufflePhase.GUESS,
    cups: cupsAt(cupCount),
    ball_cup_index: -1,
    time_left: 8000
  })
}

function showRoundResult(cupCount: number) {
  push({
    phase: ShellShufflePhase.ROUND_RESULT,
    cups: cupsAt(cupCount),
    ball_cup_index: 1,
    time_left: 4000
  })
  roundResult.value = {
    current_round: 1,
    correct_cup_index: 1,
    player_results: [
      { name: 'Alice', was_correct: true, guessed_cup: 1 },
      { name: 'Bob', was_correct: false, guessed_cup: 0 }
    ],
    players_remaining: 1
  }
}
</script>

<template>
  <div class="w-screen h-screen bg-gray-900 text-white p-4 flex flex-col gap-4">
    <h1 class="text-xl font-bold">shellShuffle migration harness (dev only)</h1>

    <div class="flex gap-2 flex-wrap items-center">
      <button class="px-3 py-1 bg-blue-600 rounded" @click="showReveal(3)">REVEAL (3 cups)</button>
      <button class="px-3 py-1 bg-blue-600 rounded" @click="showShuffle(3)">
        SHUFFLE (3 cups)
      </button>
      <button class="px-3 py-1 bg-blue-600 rounded" @click="showGuess(3)">GUESS (3 cups)</button>
      <button class="px-3 py-1 bg-blue-600 rounded" @click="showRoundResult(3)">
        ROUND_RESULT (3 cups)
      </button>
      <button class="px-3 py-1 bg-purple-600 rounded" @click="showGuess(6)">
        GUESS (6 cups, max)
      </button>
      <button class="px-3 py-1 bg-gray-600 rounded" @click="containerWide = !containerWide">
        Toggle container aspect ({{ containerWide ? 'wide' : 'tall' }})
      </button>
    </div>

    <div
      class="text-sm font-mono"
      :class="canvasElementCount === 1 ? 'text-green-400' : 'text-red-400'"
    >
      canvas elements in DOM: {{ canvasElementCount }} (re-tests the letterbox prerequisite fix —
      toggle the container aspect above and confirm the cups never clip or shift off-canvas)
    </div>

    <div
      id="harness-mount"
      class="flex-1 border border-gray-700"
      :class="containerWide ? 'w-full' : 'w-96 mx-auto'"
    >
      <ShellShuffleGameView ref="gameViewRef" :round-result="roundResult" />
    </div>
  </div>
</template>
