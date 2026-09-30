<script setup lang="ts">
import { ref, onMounted, onUnmounted } from 'vue'
import HostView from '@/components/highwayHustle/HostView.vue'
import type {
  HighwayHustleData,
  HighwayHustleEntity,
  HighwayHustleResult
} from '@/components/highwayHustle/HighwayHustleModels'

// Migration verification harness — mounts the real HostView (not the child GameView directly),
// driving it via its dev-only __devUpdateHost/__devUpdateResult hooks so the real prop-passing
// path is what's under test. Scenarios specifically target the two risks unique to this
// migration: the results-screen "hidden coupling" fix (leaderboard slot positions used to be
// populated as a side effect of the road Graphics having rendered once — now a plain constant),
// and the entity-layer pooling replacing the old dead `applicationId`-driven recreation attempt.
// Dev-only (see router/index.ts), never present in a production build.

const hostViewRef = ref<InstanceType<typeof HostView>>()
const canvasElementCount = ref(0)

function updateCanvasCount() {
  requestAnimationFrame(() => {
    canvasElementCount.value = document.querySelectorAll('#harness-mount canvas').length
  })
}

onMounted(updateCanvasCount)

// Real backend map size (HighwayHustle_Map(750, 480)).
const MAP_WIDTH = 750
const MAP_HEIGHT = 480
const PLAYER_COUNT = 4

function freshPlayers(): HighwayHustleEntity[] {
  return Array.from({ length: PLAYER_COUNT }, (_, i) => ({
    id: `Player ${i + 1}`,
    x: 40,
    y: 60 + i * 100,
    carType: i,
    isDead: false
  }))
}

let players: HighwayHustleEntity[] = freshPlayers()
let obstacles: HighwayHustleEntity[] = []
let obstacleAutoIncrement = 0
let distance = -3000 // negative distance = pre-race countdown, matches the real backend behavior

function pushHost() {
  const data: HighwayHustleData = { players, obstacles, distance }
  hostViewRef.value?.__devUpdateHost?.(data)
  updateCanvasCount()
}

function resetRace() {
  players = freshPlayers()
  obstacles = []
  obstacleAutoIncrement = 0
  distance = -3000
  pushHost()
}

let tickTimer: ReturnType<typeof setInterval> | null = null

function startRace() {
  distance = 0
  pushHost()
  tickTimer = setInterval(() => {
    distance += 100
    players = players.map((p) => (p.isDead ? p : { ...p, x: p.x + 5 }))
    pushHost()
  }, 100)
}

function stopRace() {
  if (tickTimer) clearInterval(tickTimer)
  tickTimer = null
}

// Adds one new obstacle entity mid-race — the direct replacement for the old dead
// `applicationId`-increment attempt at forcing a redraw; confirms createEntityLayer picks up a
// growing obstacle list without recreating the canvas (canvas-element-count must stay 1).
function addObstacle() {
  const id = obstacleAutoIncrement++
  obstacles = [
    ...obstacles,
    { id: `obs_${id}`, x: 700, y: 60 + (id % 6) * 70, carType: id % 15, isDead: false }
  ]
  pushHost()
}

function killAPlayer() {
  const alive = players.find((p) => !p.isDead)
  if (!alive) return
  players = players.map((p) => (p.id === alive.id ? { ...p, isDead: true } : p))
  pushHost()
}

function finishRace() {
  stopRace()
  const results: HighwayHustleResult = {
    results: players.map((p, i) => ({
      name: p.id,
      score: (PLAYER_COUNT - i) * 100,
      placement: i + 1
    }))
  }
  hostViewRef.value?.__devUpdateResult?.(results)
  updateCanvasCount()
}

// Jumps straight to the results screen with no preceding __devUpdateHost call at all — the
// specific scenario that would have broken under the old code (results sprite positions
// depended on the road Graphics having already rendered once during racing).
function resultsFromColdStart() {
  const results: HighwayHustleResult = {
    results: freshPlayers().map((p, i) => ({
      name: p.id,
      score: (PLAYER_COUNT - i) * 100,
      placement: i + 1
    }))
  }
  hostViewRef.value?.__devUpdateResult?.(results)
  updateCanvasCount()
}

onUnmounted(stopRace)
</script>

<template>
  <div class="w-screen h-screen bg-gray-900 text-white p-4 flex flex-col gap-4">
    <h1 class="text-xl font-bold">highwayHustle migration harness (dev only)</h1>

    <!-- highwayHustle's own (pre-existing, unmodified) "absolute top-0" countdown overlay has no
         positioned ancestor to contain it, so it renders relative to the viewport and can sit on
         top of this button row once mounted — z-50 + relative keeps these clickable. -->
    <div class="flex gap-2 flex-wrap items-center relative z-50">
      <button class="px-3 py-1 bg-blue-600 rounded" @click="resetRace">
        Reset (countdown, no obstacles)
      </button>
      <button class="px-3 py-1 bg-blue-600 rounded" @click="startRace">Start race</button>
      <button class="px-3 py-1 bg-gray-600 rounded" @click="stopRace">Stop race</button>
      <button class="px-3 py-1 bg-teal-600 rounded" @click="addObstacle">Add obstacle</button>
      <button class="px-3 py-1 bg-yellow-600 rounded" @click="killAPlayer">Kill a player</button>
      <button class="px-3 py-1 bg-orange-600 rounded" @click="finishRace">Finish race</button>
      <button class="px-3 py-1 bg-red-600 rounded" @click="resultsFromColdStart">
        Results from cold start (no prior race data)
      </button>
    </div>

    <div class="flex gap-6 flex-wrap text-sm font-mono">
      <div :class="canvasElementCount === 1 ? 'text-green-400' : 'text-red-400'">
        canvas elements in DOM: {{ canvasElementCount }} — must stay 1 across every scenario above.
      </div>
    </div>

    <p class="text-xs text-gray-400 max-w-3xl">
      Letterbox: resize this window / use devtools device toolbar and confirm the {{ MAP_WIDTH }}x{{
        MAP_HEIGHT
      }}
      world stays centered and undistorted (cars stay circular/rectangular, never stretched).
      Obstacles: "Add obstacle" should never recreate the canvas. Results: both "Finish race" (after
      racing) and "Results from cold start" (skipping racing entirely) must render the same correct
      leaderboard bracket layout — the cold-start case is what the hidden-coupling fix specifically
      targets.
    </p>

    <div id="harness-mount" class="flex-1 border border-gray-700">
      <HostView ref="hostViewRef" :width="800" :height="600" />
    </div>
  </div>
</template>
