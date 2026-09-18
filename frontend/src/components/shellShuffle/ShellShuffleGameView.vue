<script lang="ts" setup>
import { ref, computed } from 'vue'
import { Graphics } from 'pixi.js'
import { useGameCanvas, useSnapshotBuffer } from '@/composables/pixi'
import type {
  ShellShuffleHostData,
  ShellShuffleCup,
  ShellShuffleRoundResult
} from './ShellShuffleModels'
import { ShellShufflePhase } from './ShellShuffleModels'

const props = defineProps<{ roundResult: ShellShuffleRoundResult | null }>()

// ─── Constants (game world units — unaffected by canvas pixel size) ──────────

const GAME_MAP_WIDTH = 800
const GAME_MAP_HEIGHT = 450

const CUP_WIDTH = 88
const CUP_HEIGHT = 120
const CUP_Y = 240
const SURFACE_Y = CUP_Y + CUP_HEIGHT
const BALL_RADIUS = 34
const BALL_Y = SURFACE_Y - BALL_RADIUS - 4
const LIFT_Y = 60
const MAX_DEPTH_SCALE = 0.25
const MAX_TINT_FACTOR = 0.18 // subtle brighten/darken as a cup nears/recedes from camera during a swap

const COLOR_TABLE = 0x1a7a3c
const COLOR_TABLE_LINE = 0x155f2e
const COLOR_CUP = 0x8b5e3c
const COLOR_CUP_RIM = 0x6b3d1e
const COLOR_BALL = 0xff5500
const COLOR_BALL_SHINE = 0xff9966

interface ShellShuffleHud {
  phase: ShellShufflePhase
  time_left: number
  cups: ShellShuffleCup[]
  current_round: number
  active_players: number
}

// Shifts a color toward white as depth increases (cup nearing camera) and toward black as it
// decreases (cup receding). factor ranges [1 - MAX_TINT_FACTOR, 1 + MAX_TINT_FACTOR] since
// depth ∈ [-1, 1] — always positive as long as MAX_TINT_FACTOR < 1 (keep it that way, or the
// darken side underflows past 0 and Math.min's high-end-only clamp won't catch it).
function depthTint(color: number, depth: number): number {
  const factor = 1 + depth * MAX_TINT_FACTOR
  return (
    Math.min(255, Math.floor((color & 0xff) * factor)) |
    (Math.min(255, Math.floor(((color >> 8) & 0xff) * factor)) << 8) |
    (Math.min(255, Math.floor(((color >> 16) & 0xff) * factor)) << 16)
  )
}

// ─── Canvas + snapshot buffer ─────────────────────────────────────────────────

const containerRef = ref<HTMLElement | null>(null)
const gfx = new Graphics()

const canvas = useGameCanvas({
  container: containerRef,
  worldSize: { width: GAME_MAP_WIDTH, height: GAME_MAP_HEIGHT },
  backgroundColor: COLOR_TABLE,
  backgroundAlpha: 1,
  render: drawScene
})
canvas.root.addChild(gfx)

const buffer = useSnapshotBuffer<ShellShuffleHostData, ShellShuffleHud>(
  {
    phase: ShellShufflePhase.REVEAL,
    current_round: 0,
    time_left: 0,
    cups: [],
    ball_cup_index: -1,
    active_players: 0,
    map_width: 800
  },
  {
    invalidate: canvas.invalidate,
    toHud: (d) => ({
      phase: d.phase,
      time_left: d.time_left,
      cups: d.cups,
      current_round: d.current_round,
      active_players: d.active_players
    })
  }
)

// toHud(initial) runs synchronously inside useSnapshotBuffer before this line returns, so
// hud.value is only ever undefined at the type level, never at runtime.
const hud = computed(() => buffer.hud.value!)

function drawScene() {
  const { cups, ball_cup_index: ballIdx } = buffer.current
  gfx.clear()

  gfx.rect(0, 0, GAME_MAP_WIDTH, GAME_MAP_HEIGHT).fill(COLOR_TABLE)
  gfx.rect(0, SURFACE_Y, GAME_MAP_WIDTH, 3).fill(COLOR_TABLE_LINE)

  // Ball is only visible during REVEAL and ROUND_RESULT (ballIdx >= 0 in those phases)
  if (ballIdx >= 0 && ballIdx < cups.length) {
    const ballX = cups[ballIdx].x_pos
    gfx.circle(ballX, BALL_Y, BALL_RADIUS).fill(COLOR_BALL)
    gfx
      .circle(ballX - BALL_RADIUS * 0.25, BALL_Y - BALL_RADIUS * 0.25, BALL_RADIUS * 0.3)
      .fill({ color: COLOR_BALL_SHINE, alpha: 0.6 })
  }

  // Draw cups sorted by depth so the "farther" cup renders behind the "closer" one.
  const drawOrder = cups.map((cup, i) => ({ cup, i })).sort((a, b) => a.cup.depth - b.cup.depth)

  drawOrder.forEach(({ cup, i }) => {
    const cx = cup.x_pos
    const isLifted = ballIdx >= 0 && i === ballIdx
    const depthScale = 1 + cup.depth * MAX_DEPTH_SCALE
    const cw = CUP_WIDTH * depthScale
    const ch = CUP_HEIGHT * depthScale
    const liftOffset = isLifted ? LIFT_Y : 0
    // Anchor on the fixed center line (CUP_Y + CUP_HEIGHT / 2) instead of a fixed top, so a
    // growing cup's top rises and bottom drops symmetrically instead of only growing downward.
    const cy = CUP_Y + (CUP_HEIGHT - ch) / 2 - liftOffset
    const tintedCup = depthTint(COLOR_CUP, cup.depth)
    const tintedRim = depthTint(COLOR_CUP_RIM, cup.depth)

    gfx.roundRect(cx - cw / 2, cy, cw, ch, 8).fill(tintedCup)
    gfx
      .roundRect(
        cx - (cw / 2 + 4 * depthScale),
        cy + ch - 14 * depthScale,
        cw + 8 * depthScale,
        14 * depthScale,
        4 * depthScale
      )
      .fill(tintedRim)
    gfx
      .roundRect(
        cx - cw / 2 - 2 * depthScale,
        cy,
        cw + 4 * depthScale,
        10 * depthScale,
        4 * depthScale
      )
      .fill(tintedRim)
  })
}

// ─── HUD-derived display state ────────────────────────────────────────────────

const phaseLabel = computed(() => {
  switch (hud.value.phase) {
    case ShellShufflePhase.REVEAL:
      return 'Watch the ball!'
    case ShellShufflePhase.SHUFFLE:
      return 'Shuffling…'
    case ShellShufflePhase.GUESS:
      return 'Players, choose your cup!'
    case ShellShufflePhase.ROUND_RESULT:
      return 'Reveal!'
    default:
      return ''
  }
})

const showTimer = computed(
  () =>
    hud.value.phase === ShellShufflePhase.REVEAL ||
    hud.value.phase === ShellShufflePhase.GUESS ||
    hud.value.phase === ShellShufflePhase.ROUND_RESULT
)

const timerSeconds = computed(() => Math.max(0, Math.ceil(hud.value.time_left / 1000)))

// Sort cup indices by x position to determine left-to-right labels
const cupLabelMap = computed(() => {
  const cups = hud.value.cups
  const indices = cups.map((_, i) => i).sort((a, b) => cups[a].x_pos - cups[b].x_pos)
  const labels: Record<number, number> = {}
  indices.forEach((arrayIdx, pos) => {
    labels[arrayIdx] = pos + 1
  })
  return labels
})

const roundResultRows = computed(() => {
  if (!props.roundResult) return []
  return props.roundResult.player_results.slice().filter((a) => !a.was_correct)
})

const push = (data: ShellShuffleHostData) => buffer.push(data)

defineExpose({
  push,
  ...(import.meta.env.DEV ? { __devPush: push } : {})
})
</script>

<template>
  <div class="flex flex-col w-full h-full bg-black">
    <!-- Header bar -->
    <div class="flex items-center justify-between px-4 py-2 bg-black bg-opacity-60">
      <div class="text-white text-xl font-bold ml-2">Round {{ hud.current_round }}</div>
      <div class="text-yellow-300 text-xl font-bold">{{ phaseLabel }}</div>
      <div class="text-white text-xl font-bold mr-2">Players left: {{ hud.active_players }}</div>
    </div>

    <!-- Canvas area -->
    <div
      ref="containerRef"
      class="flex-1 overflow-hidden relative flex items-center justify-center"
    >
      <!-- Cup number labels HTML overlay (during GUESS / ROUND_RESULT) -->
      <template
        v-if="hud.phase === ShellShufflePhase.GUESS || hud.phase === ShellShufflePhase.ROUND_RESULT"
      >
        <div
          v-for="(cup, i) in hud.cups"
          :key="i"
          class="absolute flex items-center justify-center text-white font-bold rounded-full bg-black bg-opacity-60 pointer-events-none"
          :style="{
            width: `${28 * canvas.scale.value}px`,
            height: `${28 * canvas.scale.value}px`,
            left: `${canvas.offsetX.value + cup.x_pos * canvas.scale.value - 14 * canvas.scale.value}px`,
            top: `${canvas.offsetY.value + (CUP_Y - 42) * canvas.scale.value}px`,
            fontSize: `${15 * canvas.scale.value}px`
          }"
        >
          {{ cupLabelMap[i] ?? i + 1 }}
        </div>
      </template>
    </div>

    <!-- Timer bar: always rendered to keep layout stable; invisible keeps the space -->
    <div
      class="flex justify-center items-center py-2 bg-black bg-opacity-60"
      :class="{ invisible: !showTimer }"
    >
      <div class="text-5xl font-bold" :class="timerSeconds <= 3 ? 'text-red-400' : 'text-white'">
        {{ timerSeconds }}
      </div>
    </div>

    <!-- Round result overlay -->
    <div
      v-if="props.roundResult && hud.phase === ShellShufflePhase.ROUND_RESULT"
      class="absolute inset-x-0 bottom-0 bg-black bg-opacity-75 px-4 py-3"
    >
      <div class="text-white text-center text-lg font-bold mb-2">
        Ball was under cup {{ props.roundResult.correct_cup_index + 1 }} <br />
        {{ props.roundResult.players_remaining }} player(s) continue
      </div>
      <div class="flex flex-wrap justify-center gap-2">
        <div
          v-for="r in roundResultRows"
          :key="r.name"
          class="px-3 py-1 rounded-full text-sm font-bold"
          :class="r.was_correct ? 'bg-green-600 text-white' : 'bg-red-600 text-white'"
        >
          {{ r.name }} {{ r.was_correct ? '✓' : '✗' }}
        </div>
      </div>
    </div>
  </div>
</template>
