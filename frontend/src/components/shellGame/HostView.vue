<script lang="ts" setup>
import { ref, computed, watch, onUnmounted } from 'vue'
import { Application } from 'vue3-pixi'
import { Graphics } from 'pixi.js'
import { type IntroductionData } from '@/components/introduction/Introduction.vue'
import Introduction from '@/components/introduction/Introduction.vue'
import {
  GameStateType,
  MiniGameIntroductionPayload,
  type MiniGamePayloadType
} from '@/flatbuffers/messageClass'
import {
  parseShellGameHostPayload,
  parseShellGameRoundResult,
  parseShellGameResult
} from './ShellGameProcessor'
import type { ShellGameHostData, ShellGameRoundResult, ShellGameResult } from './ShellGameModels'
import { ShellGamePhase } from './ShellGameModels'

const props = defineProps<{
  width: number
  height: number
}>()

// ─── Constants ───────────────────────────────────────────────────────────────

const GAME_MAP_WIDTH = 800
const GAME_MAP_HEIGHT = 450
const MAP_ASPECT = GAME_MAP_WIDTH / GAME_MAP_HEIGHT

// Cup visual constants (in game world units)
const CUP_WIDTH = 88
const CUP_HEIGHT = 120
const CUP_Y = 240          // top of cup in game world
const SURFACE_Y = CUP_Y + CUP_HEIGHT  // = 360
const BALL_RADIUS = 34
const BALL_Y = SURFACE_Y - BALL_RADIUS - 4  // sits just above the surface line

// Colors
const COLOR_TABLE = 0x1a7a3c
const COLOR_TABLE_LINE = 0x155f2e
const COLOR_CUP = 0x8b5e3c
const COLOR_CUP_RIM = 0x6b3d1e
const COLOR_BALL = 0xff5500
const COLOR_BALL_SHINE = 0xff9966

// ─── State ───────────────────────────────────────────────────────────────────

enum ViewState {
  None,
  Introduction,
  MiniGame,
  Results
}

const viewState = ref<ViewState>(ViewState.None)
const intro = ref<IntroductionData>({ title: '', description: '', time_left: 0 })

const hostData = ref<ShellGameHostData>({
  phase: ShellGamePhase.REVEAL,
  current_round: 0,
  time_left: 0,
  cups: [],
  ball_cup_index: -1,
  active_players: 0,
  map_width: 800
})

const roundResult = ref<ShellGameRoundResult | null>(null)
const gameResult = ref<ShellGameResult | null>(null)

// ─── Canvas sizing ────────────────────────────────────────────────────────────
// Constrain by whichever dimension is the bottleneck so the canvas never
// overflows its container — this handles both narrow and ultrawide screens.

const canvasContainerRef = ref<HTMLElement | null>(null)
const containerWidth = ref(780)
const containerHeight = ref(440)

const canvasWidth = computed(() =>
  Math.min(containerWidth.value, Math.round(containerHeight.value * MAP_ASPECT))
)
const canvasHeight = computed(() => Math.round(canvasWidth.value / MAP_ASPECT))
const scale = computed(() => canvasWidth.value / GAME_MAP_WIDTH)

let resizeObserver: ResizeObserver | null = null
watch(canvasContainerRef, (el) => {
  resizeObserver?.disconnect()
  if (el) {
    containerWidth.value = el.clientWidth
    containerHeight.value = el.clientHeight
    resizeObserver = new ResizeObserver((entries) => {
      containerWidth.value = entries[0].contentRect.width
      containerHeight.value = entries[0].contentRect.height
    })
    resizeObserver.observe(el)
  }
})
onUnmounted(() => resizeObserver?.disconnect())

// ─── Helpers ─────────────────────────────────────────────────────────────────

const s = scale

// Offset of the canvas within its flex-centered container.
// HTML overlays positioned absolute inside the container must add this offset
// so they align with what is drawn inside the PixiJS canvas.
const canvasOffsetX = computed(() => (containerWidth.value - canvasWidth.value) / 2)
const canvasOffsetY = computed(() => (containerHeight.value - canvasHeight.value) / 2)

const phaseLabel = computed(() => {
  switch (hostData.value.phase) {
    case ShellGamePhase.REVEAL: return 'Watch the ball!'
    case ShellGamePhase.SHUFFLE: return 'Shuffling…'
    case ShellGamePhase.GUESS: return 'Players, choose your cup!'
    case ShellGamePhase.ROUND_RESULT: return 'Reveal!'
    default: return ''
  }
})

const showTimer = computed(
  () =>
    hostData.value.phase === ShellGamePhase.REVEAL ||
    hostData.value.phase === ShellGamePhase.GUESS ||
    hostData.value.phase === ShellGamePhase.ROUND_RESULT
)

const timerSeconds = computed(() =>
  Math.max(0, Math.ceil(hostData.value.time_left / 1000))
)

// Sort cup indices by x position to determine left-to-right labels
const cupLabelMap = computed(() => {
  const cups = hostData.value.cups
  const indices = cups.map((_, i) => i).sort((a, b) => cups[a].x_pos - cups[b].x_pos)
  const labels: Record<number, number> = {}
  indices.forEach((arrayIdx, pos) => {
    labels[arrayIdx] = pos + 1   // 1-based label
  })
  return labels
})

// ─── PixiJS rendering ────────────────────────────────────────────────────────

const renderGame = (g: Graphics) => {
  g.clear()
  const sv = s.value
  const cups = hostData.value.cups
  const ballIdx = hostData.value.ball_cup_index

  const w = canvasWidth.value
  const h = canvasHeight.value

  // Background table
  g.beginFill(COLOR_TABLE)
  g.drawRect(0, 0, w, h)
  g.endFill()

  // Surface line
  g.beginFill(COLOR_TABLE_LINE)
  g.drawRect(0, SURFACE_Y * sv, w, 3 * sv)
  g.endFill()

  // Ball is only visible during REVEAL and ROUND_RESULT (ballIdx >= 0 in those phases)
  if (ballIdx >= 0 && ballIdx < cups.length) {
    const ballX = cups[ballIdx].x_pos * sv
    const bY = BALL_Y * sv
    const br = BALL_RADIUS * sv
    g.beginFill(COLOR_BALL)
    g.drawCircle(ballX, bY, br)
    g.endFill()
    g.beginFill(COLOR_BALL_SHINE, 0.6)
    g.drawCircle(ballX - br * 0.25, bY - br * 0.25, br * 0.3)
    g.endFill()
  }

  // Draw cups — lift the ball cup upward during REVEAL and ROUND_RESULT
  const LIFT_Y = 60  // pixels to lift the revealed cup
  cups.forEach((cup, i) => {
    const cx = cup.x_pos * sv
    const isLifted = ballIdx >= 0 && i === ballIdx
    const cy = (isLifted ? CUP_Y - LIFT_Y : CUP_Y) * sv
    const cw = CUP_WIDTH * sv
    const ch = CUP_HEIGHT * sv

    // Cup body
    g.beginFill(COLOR_CUP)
    g.drawRoundedRect(cx - cw / 2, cy, cw, ch, 8 * sv)
    g.endFill()

    // Cup rim (wider band at the bottom)
    g.beginFill(COLOR_CUP_RIM)
    g.drawRoundedRect(cx - (cw / 2 + 4 * sv), cy + ch - 14 * sv, cw + 8 * sv, 14 * sv, 4 * sv)
    g.endFill()

    // Cup top cap
    g.beginFill(COLOR_CUP_RIM)
    g.drawRoundedRect(cx - cw / 2 - 2 * sv, cy, cw + 4 * sv, 10 * sv, 4 * sv)
    g.endFill()

    // Number label background circle (during GUESS and ROUND_RESULT)
    // if (phase === ShellGamePhase.GUESS || phase === ShellGamePhase.ROUND_RESULT) {
    //   const label = cupLabelMap.value[i]
    //   if (label !== undefined) {
    //     g.beginFill(0x000000, 0.5)
    //     g.drawCircle(cx, (CUP_Y - 32) * sv, 16 * sv)
    //     g.endFill()
    //   }
    // }
  })
}

// ─── Round result overlay ────────────────────────────────────────────────────

const roundResultRows = computed(() => {
  if (!roundResult.value) return []
  return roundResult.value.player_results
    .slice()
    .sort((a, b) => (b.was_correct ? 1 : 0) - (a.was_correct ? 1 : 0))
})

// ─── Ordinal formatting ───────────────────────────────────────────────────────

const pr = new Intl.PluralRules('en-US', { type: 'ordinal' })
const suffixes = new Map([['one', 'st'], ['two', 'nd'], ['few', 'rd'], ['other', 'th']])
const formatOrdinals = (n: number) => `${n}${suffixes.get(pr.select(n))}`

// ─── Update handler ───────────────────────────────────────────────────────────

const update = (data: MiniGamePayloadType) => {
  switch (data.gamestatetype()) {
    case GameStateType.ShellGameHost: {
      viewState.value = ViewState.MiniGame
      hostData.value = parseShellGameHostPayload(data)
      // Clear round result overlay when leaving ROUND_RESULT phase
      if (hostData.value.phase !== ShellGamePhase.ROUND_RESULT) {
        roundResult.value = null
      }
      break
    }
    case GameStateType.ShellGameRoundResult: {
      roundResult.value = parseShellGameRoundResult(data)
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
  <template v-if="viewState === ViewState.Introduction">
    <Introduction logoSVG="/assets/games/shellGame/shellGameLogo.svg" :data="intro" />
  </template>

  <template v-else-if="viewState === ViewState.MiniGame">
    <div class="flex flex-col w-full h-full bg-black">
      <!-- Header bar -->
      <div class="flex items-center justify-between px-4 py-2 bg-black bg-opacity-60">
        <div class="text-white text-xl font-bold">Round {{ hostData.current_round }}</div>
        <div class="text-yellow-300 text-xl font-bold">{{ phaseLabel }}</div>
        <div class="text-white text-xl font-bold">
          Players: {{ hostData.active_players }}
        </div>
      </div>

      <!-- Canvas area -->
      <div ref="canvasContainerRef" class="flex-1 overflow-hidden relative flex items-center justify-center">
        <Application
          v-if="canvasWidth > 0"
          :width="canvasWidth"
          :height="canvasHeight"
          :backgroundAlpha="1"
          :backgroundColor="0x1a7a3c"
        >
          <Graphics @render="renderGame" />
        </Application>

        <!-- Cup number labels HTML overlay (during GUESS / ROUND_RESULT) -->
        <template v-if="hostData.phase === ShellGamePhase.GUESS || hostData.phase === ShellGamePhase.ROUND_RESULT">
          <div
            v-for="(cup, i) in hostData.cups"
            :key="i"
            class="absolute flex items-center justify-center text-white font-bold rounded-full bg-black bg-opacity-60 pointer-events-none"
            :style="{
              width: `${28 * s}px`,
              height: `${28 * s}px`,
              left: `${canvasOffsetX + cup.x_pos * s - 14 * s}px`,
              top: `${canvasOffsetY + (CUP_Y - 42) * s}px`,
              fontSize: `${15 * s}px`
            }"
          >
            {{ cupLabelMap[i] ?? i + 1 }}
          </div>
        </template>
      </div>

      <!-- Timer bar: always rendered to keep layout stable; invisible keeps the space -->
      <div class="flex justify-center items-center py-2 bg-black bg-opacity-60" :class="{ invisible: !showTimer }">
        <div
          class="text-5xl font-bold"
          :class="timerSeconds <= 3 ? 'text-red-400' : 'text-white'"
        >
          {{ timerSeconds }}
        </div>
      </div>

      <!-- Round result overlay -->
      <div
        v-if="roundResult && hostData.phase === ShellGamePhase.ROUND_RESULT"
        class="absolute inset-x-0 bottom-0 bg-black bg-opacity-75 px-4 py-3"
      >
        <div class="text-white text-center text-lg font-bold mb-2">
          Ball was under cup {{ roundResult.correct_cup_index + 1 }} —
          {{ roundResult.players_remaining }} player(s) continue
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

  <template v-else-if="viewState === ViewState.Results">
    <div class="flex flex-col w-full h-full bg-black text-white p-6 gap-4">
      <div class="text-center text-4xl font-bold text-yellow-300 mb-2">Final Results</div>
      <div class="flex flex-col gap-3 overflow-y-auto">
        <div
          v-for="r in gameResult?.results"
          :key="r.name"
          class="flex items-center justify-between bg-white bg-opacity-10 rounded-xl px-6 py-3"
        >
          <div class="text-3xl font-bold text-yellow-300 w-16">
            {{ formatOrdinals(r.placement) }}
          </div>
          <div class="text-2xl font-semibold flex-1 text-center">{{ r.name }}</div>
          <div class="text-lg text-gray-300 text-right">
            {{ r.rounds_survived }} round{{ r.rounds_survived !== 1 ? 's' : '' }}
          </div>
        </div>
      </div>
    </div>
  </template>
</template>
