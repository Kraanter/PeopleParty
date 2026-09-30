<script lang="ts" setup>
import { ref, watch } from 'vue'
import { Graphics } from 'pixi.js'
import { useGameCanvas, createEntityLayer, useSnapshotBuffer } from '@/composables/pixi'
import type { TeambasedPongHostData } from './TeambasedPongModels'

const props = defineProps<{ hostData: TeambasedPongHostData }>()

// Real backend constant (teambased_pong_map.h:38-39, MAP_WIDTH/MAP_HEIGHT static constexpr) —
// no camera here, the whole court is always fully in frame, letterboxed to fill as much of the
// screen as possible without distorting the (circular) ball/markers.
const WORLD_WIDTH = 800
const WORLD_HEIGHT = 600

// Backend pushes at this cadence — see the note on getSmoothedDirection below for why this
// matters for frame-rate-independent smoothing.
const NETWORK_TICK_MS = 50
const INPUT_SMOOTHING_FACTOR = 0.2

const containerRef = ref<HTMLElement | null>(null)
const centerLineGfx = new Graphics()
const paddleAGfx = new Graphics()
const paddleBGfx = new Graphics()
const ballGfx = new Graphics()

const canvas = useGameCanvas({
  container: containerRef,
  worldSize: { width: WORLD_WIDTH, height: WORLD_HEIGHT },
  backgroundColor: 0x000000,
  backgroundAlpha: 1,
  render: () => drawScene()
})
canvas.root.addChild(centerLineGfx, paddleAGfx, paddleBGfx, ballGfx)

// Center line is static geometry — draw once, not every frame.
for (let y = 0; y < WORLD_HEIGHT; y += 30) {
  centerLineGfx.moveTo(WORLD_WIDTH / 2, y)
  centerLineGfx.lineTo(WORLD_WIDTH / 2, y + 15)
}
centerLineGfx.stroke({ width: 3, color: 0xffffff, alpha: 0.5 })

interface MarkerEntity {
  key: string
  x: number
  y: number
}

function createMarkerLayer(color: number) {
  return createEntityLayer<MarkerEntity, Graphics>(canvas.root, {
    key: (marker) => marker.key,
    create: () => new Graphics(),
    update: (gfx, marker) => {
      gfx.clear()
      gfx.circle(marker.x, marker.y, 8).fill(color)
    }
  })
}

const teamAMarkerLayer = createMarkerLayer(0x00ff00)
const teamBMarkerLayer = createMarkerLayer(0xff0000)

const buffer = useSnapshotBuffer<TeambasedPongHostData>(props.hostData, {
  invalidate: canvas.invalidate
})

// Per-player direction smoothing state, to avoid marker teleporting between network ticks.
// Pre-migration, this factor was applied once per incoming network message (~NETWORK_TICK_MS)
// — correct convergence, but no actual per-frame interpolation, so it only ever "snapped" on
// each new message despite the comment claiming smoothing. Recalibrated here to run once per
// *rendered frame* via dt, using the same time-reference recalibration marbleMania's camera-Y
// smoothing needed for the identical reason (a constant tuned for "once per message" produces a
// snap-then-pause feel if reused directly inside a per-frame formula at a different cadence).
const smoothedTeamAInputs = new Map<string, number>()
const smoothedTeamBInputs = new Map<string, number>()

const clampDirection = (value: number) => Math.max(0, Math.min(100, value))

function getSmoothedDirection(
  smoothingMap: Map<string, number>,
  playerKey: string,
  targetDirection: number,
  dt: number
): number {
  const target = clampDirection(targetDirection)
  const current = smoothingMap.get(playerKey) ?? target
  const easeFactor = 1 - Math.pow(1 - INPUT_SMOOTHING_FACTOR, dt / NETWORK_TICK_MS)
  const next = current + (target - current) * easeFactor
  smoothingMap.set(playerKey, next)
  return next
}

function pruneMissingPlayers(smoothingMap: Map<string, number>, activePlayerKeys: Set<string>) {
  for (const key of smoothingMap.keys()) {
    if (!activePlayerKeys.has(key)) smoothingMap.delete(key)
  }
}

let paddleAX = 0
let paddleAY = 0
let paddleBX = 0
let paddleBY = 0
let ballX = 0
let ballY = 0
let paddleWidth = 20
let paddleHeight = 120
let teamAMarkers: MarkerEntity[] = []
let teamBMarkers: MarkerEntity[] = []

canvas.startAnimating(({ dt }) => {
  const data = buffer.current
  const mw = data.map_width
  const mh = data.map_height
  paddleWidth = data.paddle_width
  paddleHeight = data.paddle_height

  paddleAX = data.paddle_a_x + mw / 2
  paddleAY = data.paddle_a_y + mh / 2
  paddleBX = data.paddle_b_x + mw / 2
  paddleBY = data.paddle_b_y + mh / 2
  ballX = data.ball_x + mw / 2
  ballY = data.ball_y + mh / 2

  pruneMissingPlayers(smoothedTeamAInputs, new Set(data.team_a_players.map((p) => p.name)))
  pruneMissingPlayers(smoothedTeamBInputs, new Set(data.team_b_players.map((p) => p.name)))

  const halfPaddleHeight = paddleHeight / 2

  teamAMarkers = []
  for (const player of data.team_a_players) {
    const smoothed = getSmoothedDirection(smoothedTeamAInputs, player.name, player.direction, dt)
    if (Math.abs(smoothed - 50) < 0.5) continue // skip near-neutral input
    teamAMarkers.push({
      key: player.name,
      x: paddleAX - 20,
      y: paddleAY + ((smoothed - 50) / 50) * halfPaddleHeight
    })
  }

  teamBMarkers = []
  for (const player of data.team_b_players) {
    const smoothed = getSmoothedDirection(smoothedTeamBInputs, player.name, player.direction, dt)
    if (Math.abs(smoothed - 50) < 0.5) continue // skip near-neutral input
    teamBMarkers.push({
      key: player.name,
      x: paddleBX + 20,
      y: paddleBY + ((smoothed - 50) / 50) * halfPaddleHeight
    })
  }
})

function drawScene() {
  paddleAGfx.clear()
  paddleAGfx
    .rect(paddleAX - paddleWidth / 2, paddleAY - paddleHeight / 2, paddleWidth, paddleHeight)
    .fill(0x00ff00)

  paddleBGfx.clear()
  paddleBGfx
    .rect(paddleBX - paddleWidth / 2, paddleBY - paddleHeight / 2, paddleWidth, paddleHeight)
    .fill(0xff0000)

  ballGfx.clear()
  ballGfx.circle(ballX, ballY, 10).fill(0xffffff)

  teamAMarkerLayer.sync(teamAMarkers)
  teamBMarkerLayer.sync(teamBMarkers)
}

watch(
  () => props.hostData,
  (data) => buffer.push(data),
  { immediate: true }
)

defineExpose({
  ...(import.meta.env.DEV
    ? {
        __devSmoothedDirection: (team: 'a' | 'b', name: string) =>
          (team === 'a' ? smoothedTeamAInputs : smoothedTeamBInputs).get(name)
      }
    : {})
})
</script>

<template>
  <div ref="containerRef" class="w-full h-full flex items-center justify-center"></div>
</template>
