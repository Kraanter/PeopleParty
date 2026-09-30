<script setup lang="ts">
import { ref, onMounted, onUnmounted } from 'vue'
import HostView from '@/components/teambasedPong/HostView.vue'
import type {
  TeambasedPongHostData,
  TeambasedPongRoundPrepData,
  TeamPlayer
} from '@/components/teambasedPong/TeambasedPongModels'

// Migration verification harness — mounts the real HostView (not a child GameView directly),
// driving it via its dev-only __devUpdateRoundPrep/__devUpdateHost hooks. Targets the two risks
// unique to this migration: the RoundPrep <-> MiniGame split into two separately-mounted
// components (useGameCanvas requires its container to exist at mount, so the two canvases can no
// longer share one component behind v-if branches — confirm both mount/unmount cleanly on every
// transition, not just once), and the dt-recalibrated per-player direction smoothing (sampled
// live below to confirm gradual convergence rather than a "snap then pause" jump).
// Dev-only (see router/index.ts), never present in a production build.

const hostViewRef = ref<InstanceType<typeof HostView>>()
const canvasElementCount = ref(0)
const directionReadout = ref(0)
let readoutRaf: number | null = null

function updateCanvasCount() {
  requestAnimationFrame(() => {
    canvasElementCount.value = document.querySelectorAll('#harness-mount canvas').length
  })
}

function pollReadouts() {
  directionReadout.value = hostViewRef.value?.__devSmoothedDirection?.('a', 'Player 1') ?? 0
  readoutRaf = requestAnimationFrame(pollReadouts)
}

onMounted(() => {
  updateCanvasCount()
  pollReadouts()
})
onUnmounted(() => {
  if (readoutRaf !== null) cancelAnimationFrame(readoutRaf)
})

const TEAM_SIZE = 3

function freshTeam(prefix: string): TeamPlayer[] {
  return Array.from({ length: TEAM_SIZE }, (_, i) => ({
    name: `${prefix} ${i + 1}`,
    direction: 50
  }))
}

function freshRoundPrepTeam(prefix: string) {
  return Array.from({ length: TEAM_SIZE }, (_, i) => ({ name: `${prefix} ${i + 1}` }))
}

function goToRoundPrep() {
  const data: TeambasedPongRoundPrepData = {
    current_round: 1,
    time_left: 3000,
    team_a_players: freshRoundPrepTeam('Player'),
    team_b_players: freshRoundPrepTeam('Opponent')
  }
  hostViewRef.value?.__devUpdateRoundPrep?.(data)
  updateCanvasCount()
}

let teamA: TeamPlayer[] = freshTeam('Player')
let teamB: TeamPlayer[] = freshTeam('Opponent')
let ballX = 0
let ballY = 0

function pushHost() {
  const data: TeambasedPongHostData = {
    current_round: 1,
    time_left: 30000,
    map_width: 800,
    map_height: 600,
    ball_x: ballX,
    ball_y: ballY,
    paddle_a_x: -370,
    paddle_a_y: 0,
    paddle_b_x: 370,
    paddle_b_y: 0,
    paddle_width: 20,
    paddle_height: 120,
    team_a_players: teamA,
    team_b_players: teamB
  }
  hostViewRef.value?.__devUpdateHost?.(data)
  updateCanvasCount()
}

function goToMiniGame() {
  pushHost()
}

let tickTimer: ReturnType<typeof setInterval> | null = null

// Player 1 (team A) jumps hard right on every tick — the exact case that would "snap then
// pause" under a smoothing factor tuned for the ~50ms message cadence but reused directly
// inside a per-rendered-frame (~16ms) formula, instead of converging gradually.
function startJoystickJitter() {
  tickTimer = setInterval(() => {
    teamA = teamA.map((p, i) => (i === 0 ? { ...p, direction: Math.random() < 0.5 ? 5 : 95 } : p))
    ballX = Math.sin(Date.now() / 500) * 300
    ballY = Math.cos(Date.now() / 700) * 200
    pushHost()
  }, 50) // matches the backend's real network tick (NETWORK_TICK_MS in TeambasedPongGameView.vue)
}

function stopJoystickJitter() {
  if (tickTimer) clearInterval(tickTimer)
  tickTimer = null
}

// Repeatedly bounces RoundPrep <-> MiniGame — the transition that requires the two-canvas split,
// since useGameCanvas needs its container to already exist at mount (a container behind a
// v-if branch that starts hidden can't work, which is why this could no longer be one component
// with two internal <Application>s the way the pre-migration code had it).
let bounceTimer: ReturnType<typeof setInterval> | null = null
let bounceToPrep = true

function startBouncing() {
  bounceTimer = setInterval(() => {
    if (bounceToPrep) goToRoundPrep()
    else goToMiniGame()
    bounceToPrep = !bounceToPrep
  }, 400)
}

function stopBouncing() {
  if (bounceTimer) clearInterval(bounceTimer)
  bounceTimer = null
}

onUnmounted(() => {
  stopJoystickJitter()
  stopBouncing()
})
</script>

<template>
  <div class="w-screen h-screen bg-gray-900 text-white p-4 flex flex-col gap-4">
    <h1 class="text-xl font-bold">teambasedPong migration harness (dev only)</h1>

    <div class="flex gap-2 flex-wrap items-center">
      <button class="px-3 py-1 bg-blue-600 rounded" @click="goToRoundPrep">Go to RoundPrep</button>
      <button class="px-3 py-1 bg-blue-600 rounded" @click="goToMiniGame">Go to MiniGame</button>
      <button class="px-3 py-1 bg-teal-600 rounded" @click="startJoystickJitter">
        Start joystick jitter (Player 1)
      </button>
      <button class="px-3 py-1 bg-gray-600 rounded" @click="stopJoystickJitter">Stop jitter</button>
      <button class="px-3 py-1 bg-purple-600 rounded" @click="startBouncing">
        Bounce RoundPrep &lt;-&gt; MiniGame
      </button>
      <button class="px-3 py-1 bg-gray-600 rounded" @click="stopBouncing">Stop bouncing</button>
    </div>

    <div class="flex gap-6 flex-wrap text-sm font-mono">
      <div :class="canvasElementCount <= 1 ? 'text-green-400' : 'text-red-400'">
        canvas elements in DOM: {{ canvasElementCount }} — must never exceed 1 (RoundPrep and
        MiniGame canvases are never both mounted at once).
      </div>
      <div>Player 1 smoothed direction (live): {{ directionReadout.toFixed(1) }}</div>
    </div>

    <p class="text-xs text-gray-400 max-w-3xl">
      Smoothing: with "Start joystick jitter" running, watch the live readout above move gradually
      toward each new target (5 or 95) across several frames rather than jumping most of the way in
      one tick and flatlining until the next message. Mount/unmount: "Bounce RoundPrep &lt;-&gt;
      MiniGame" repeatedly mounts/unmounts each canvas-owning component — confirm no console errors
      and the canvas count above never exceeds 1.
    </p>

    <div id="harness-mount" class="flex-1 border border-gray-700">
      <HostView ref="hostViewRef" :width="800" :height="600" />
    </div>
  </div>
</template>
