<script setup lang="ts">
import { ref, onMounted, onUnmounted } from 'vue'
import HostView from '@/components/marbleMania/HostView.vue'
import type { MarbleManiaData, MarbleManiaEntity } from '@/components/marbleMania/MarbleManiaModels'

// Bug-fix verification harness — mounts the real HostView (not the child directly), driving it
// via its dev-only __devUpdateHost hook so the real prop-passing path (the thing that was
// actually broken) is what's under test, not bypassed. Scenarios deliberately model the
// backend's real behavior rather than convenient test data: obstacles included in exactly one
// message, a specific marble finishing while still leading, an all-finished tail, and a
// Pause/Resume-style resend. Dev-only (see router/index.ts), never present in a production build.

const hostViewRef = ref<InstanceType<typeof HostView>>()
const canvasElementCount = ref(0)
const cameraYReadout = ref(0)
const marble1PosReadout = ref({ x: 0, y: 0 })
let readoutRaf: number | null = null

function updateCanvasCount() {
  requestAnimationFrame(() => {
    canvasElementCount.value = document.querySelectorAll('#harness-mount canvas').length
  })
}

function pollReadouts() {
  cameraYReadout.value = hostViewRef.value?.__devCameraY?.() ?? 0
  marble1PosReadout.value = hostViewRef.value?.__devMarblePos?.('marble_1') ?? { x: 0, y: 0 }
  readoutRaf = requestAnimationFrame(pollReadouts)
}

onMounted(() => {
  updateCanvasCount()
  pollReadouts()
})
onUnmounted(() => {
  if (readoutRaf !== null) cancelAnimationFrame(readoutRaf)
})

const WORLD_MIN = { x: -400, y: -300 }
const WORLD_MAX = { x: 400, y: 5000 }
const MARBLE_COUNT = 6
const OBSTACLE_COUNT = 60 // realistic order-of-magnitude count — exercises culling, not just a handful

function freshObstacles(): MarbleManiaEntity[] {
  return Array.from({ length: OBSTACLE_COUNT }, (_, i) => ({
    id: `obs_${i}`,
    type: 'obstacle',
    pos: { x: (Math.random() * 2 - 1) * 380, y: 200 + i * 75 },
    rotation: Math.random() * Math.PI,
    finished: false,
    shape: i % 3 === 0 ? { kind: 'circle', radius: 20 } : { kind: 'rect', width: 60, height: 15 }
  }))
}

function freshMarbles(): MarbleManiaEntity[] {
  return Array.from({ length: MARBLE_COUNT }, (_, i) => ({
    id: `marble_${i + 1}`,
    type: 'marble',
    pos: { x: (i - MARBLE_COUNT / 2) * 90, y: WORLD_MIN.y + 50 },
    rotation: 0,
    finished: false,
    player_name: `Player ${i + 1}`,
    shape: { kind: 'circle', radius: 15 }
  }))
}

let marbles: MarbleManiaEntity[] = freshMarbles()
// Mirrors the backend's own `sended_obstacles` flag exactly: true once obstacles have been
// included in one outgoing message, only reset by the "Simulate resume" scenario below.
let sendObstacles = true

function pushSnapshot(phase: number) {
  const entities: MarbleManiaEntity[] = sendObstacles
    ? [...freshObstacles(), ...marbles]
    : [...marbles]
  sendObstacles = false
  const data: MarbleManiaData = {
    entities,
    game_phase: phase,
    placement_time_left: phase === 0 ? 10 : 0,
    finish_line_y: WORLD_MAX.y - 200,
    world_min: WORLD_MIN,
    world_max: WORLD_MAX
  }
  hostViewRef.value?.__devUpdateHost?.(data)
  updateCanvasCount()
}

function resetGame() {
  marbles = freshMarbles()
  sendObstacles = true
  pushSnapshot(0)
}

let tickTimer: ReturnType<typeof setInterval> | null = null
let marble1BounceTicksRemaining = 0

function startSimulation() {
  pushSnapshot(1)
  tickTimer = setInterval(() => {
    marbles = marbles.map((m) => {
      if (m.finished) return m
      if (m.id === 'marble_1' && marble1BounceTicksRemaining > 0) {
        marble1BounceTicksRemaining--
        return { ...m, pos: { x: m.pos.x, y: m.pos.y - 40 } } // sharp reversal, simulating a bounce
      }
      return { ...m, pos: { x: m.pos.x, y: m.pos.y + 15 } }
    })
    pushSnapshot(1)
  }, 50) // matches the backend's real ~50ms (target_fps=20) send interval
}

// Sharply reverses marble_1's y-direction for 3 ticks (simulating a Box2D bounce off an
// obstacle), then lets it resume falling normally on the next tick. None of the other scenarios
// can exercise this — they only ever move marbles monotonically or freeze them — but this is
// exactly the case that broke the old exponential-ease smoothing (the display kept easing along
// the pre-bounce direction for a few frames, floating past where the true position had already
// reversed). Requires "Start simulation" to already be running.
function simulateBounce() {
  marble1BounceTicksRemaining = 3
}

function stopSimulation() {
  if (tickTimer) clearInterval(tickTimer)
  tickTimer = null
}

// Specifically targets the camera-freeze bug: advances marbles[0] — the one a no-seed reduce
// would silently start from — well ahead of the pack, then finishes it while still in the lead.
function finishLeaderWhileAhead() {
  marbles = marbles.map((m, i) =>
    i === 0 ? { ...m, pos: { x: m.pos.x, y: m.pos.y + 800 }, finished: true } : m
  )
  pushSnapshot(1)
}

function finishOneMarble() {
  const next = marbles.find((m) => !m.finished)
  if (!next) return
  marbles = marbles.map((m) => (m.id === next.id ? { ...m, finished: true } : m))
  pushSnapshot(1)
}

// The ~2s "everyone finished" tail every real race passes through before the results screen —
// the sole driver of camera behavior for that whole window (see plan's Fix 2 notes).
function finishEveryone() {
  marbles = marbles.map((m) => ({ ...m, finished: true }))
  pushSnapshot(2)
}

// Mirrors the backend's resume() resetting sended_obstacles=false — confirms the union-merge
// picks obstacles back up rather than silently discarding the resend (the second bug in Fix 1).
function simulateResume() {
  sendObstacles = true
  pushSnapshot(0)
}

onUnmounted(stopSimulation)
</script>

<template>
  <div class="w-screen h-screen bg-gray-900 text-white p-4 flex flex-col gap-4">
    <h1 class="text-xl font-bold">marbleMania bug-fix harness (dev only)</h1>

    <div class="flex gap-2 flex-wrap items-center">
      <button class="px-3 py-1 bg-blue-600 rounded" @click="resetGame">
        Reset game (first message, obstacles included)
      </button>
      <button class="px-3 py-1 bg-blue-600 rounded" @click="startSimulation">
        Start simulation
      </button>
      <button class="px-3 py-1 bg-gray-600 rounded" @click="stopSimulation">Stop simulation</button>
      <button class="px-3 py-1 bg-teal-600 rounded" @click="simulateBounce">
        Simulate bounce (marble_1)
      </button>
      <button class="px-3 py-1 bg-yellow-600 rounded" @click="finishOneMarble">
        Finish one marble
      </button>
      <button class="px-3 py-1 bg-orange-600 rounded" @click="finishLeaderWhileAhead">
        Finish leader while ahead
      </button>
      <button class="px-3 py-1 bg-red-600 rounded" @click="finishEveryone">
        Finish everyone (game_phase 2)
      </button>
      <button class="px-3 py-1 bg-purple-600 rounded" @click="simulateResume">
        Simulate Pause→Resume (resend obstacles)
      </button>
    </div>

    <div class="flex gap-6 flex-wrap text-sm font-mono">
      <div :class="canvasElementCount === 1 ? 'text-green-400' : 'text-red-400'">
        canvas elements in DOM: {{ canvasElementCount }} — must stay 1 across every scenario above.
      </div>
      <div>camera smoothedCameraY (live): {{ cameraYReadout.toFixed(1) }}</div>
      <div>
        marble_1 render pos (live): x={{ marble1PosReadout.x.toFixed(1) }} y={{
          marble1PosReadout.y.toFixed(1)
        }}
      </div>
    </div>

    <p class="text-xs text-gray-400 max-w-3xl">
      Obstacles: "Reset game" is the only click that ever sends them (matches the backend's one-shot
      `sended_obstacles`) — confirm they render and then keep rendering through subsequent clicks
      that don't resend them. Camera freeze: "Finish leader while ahead" should NOT stop the camera
      from following the remaining marbles. Smoothness: watch the live `smoothedCameraY`/marble_1
      readouts change gradually across consecutive "Start simulation" ticks rather than jumping most
      of the way in one tick and flatlining. Bounce: with `MARBLE_SMOOTHING = true`, click "Simulate
      bounce" mid-simulation and watch the marble_1 y readout — it should track the reversal
      smoothly and stay within the range of its actual positions before/after, never overshooting
      past where the true position turned around.
    </p>

    <div id="harness-mount" class="flex-1 border border-gray-700">
      <HostView ref="hostViewRef" :width="800" :height="600" />
    </div>
  </div>
</template>
