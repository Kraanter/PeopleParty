<script setup lang="ts">
import { ref, onMounted, onUnmounted } from 'vue'
import { Graphics } from 'pixi.js'
import { useGameCanvas, createEntityLayer, useSnapshotBuffer } from '@/composables/pixi'

// Temporary Phase 1 verification harness for the new PixiJS composables — not part of any
// real game flow, dev-only (see router/index.ts). Exercises all three composables together
// against synthetic bouncing balls so the plan's three verification targets are objectively
// checkable on screen rather than just eyeballed: redraws stop while idle, entity-count
// changes never recreate the Application/canvas, and dt-based smoothing looks consistent
// regardless of refresh rate.

interface Ball {
  id: string
  x: number
  y: number
  color: number
}

const WORLD_WIDTH = 800
const WORLD_HEIGHT = 450
const BALL_RADIUS = 20

const containerRef = ref<HTMLElement | null>(null)

const renderCount = ref(0)
const rendersSinceLastTick = ref(0)
const tickCount = ref(0)
const canvasElementCount = ref(0)
const isAnimating = ref(false)

const canvas = useGameCanvas({
  container: containerRef,
  worldSize: { width: WORLD_WIDTH, height: WORLD_HEIGHT },
  backgroundColor: 0x1a1a2e,
  render: () => {
    renderCount.value++
    rendersSinceLastTick.value++
    layer.sync(buffer.current.balls)
    canvasElementCount.value = containerRef.value?.querySelectorAll('canvas').length ?? 0
  }
})

const buffer = useSnapshotBuffer<{ balls: Ball[] }>(
  { balls: [] },
  { invalidate: canvas.invalidate }
)

const layer = createEntityLayer<Ball, Graphics>(canvas.root, {
  key: (ball) => ball.id,
  create: () => new Graphics(),
  update: (g, ball) => {
    g.clear()
    g.circle(0, 0, BALL_RADIUS).fill(ball.color)
    g.position.set(ball.x, ball.y)
  }
})

let nextBallSeq = 1
const ballIds = ref<string[]>([])
const targets = new Map<string, { x: number; y: number; color: number }>()
const smoothed = new Map<string, { x: number; y: number }>()

function randomTarget() {
  return {
    x: BALL_RADIUS * 2 + Math.random() * (WORLD_WIDTH - BALL_RADIUS * 4),
    y: BALL_RADIUS * 2 + Math.random() * (WORLD_HEIGHT - BALL_RADIUS * 4),
    color: Math.floor(Math.random() * 0xffffff)
  }
}

function pushSnapshotFromTargets() {
  rendersSinceLastTick.value = 0
  const balls: Ball[] = ballIds.value.map((id) => {
    const target = targets.get(id)!
    return { id, x: target.x, y: target.y, color: target.color }
  })
  buffer.push({ balls })
}

function addBall() {
  const id = `ball-${nextBallSeq++}`
  ballIds.value.push(id)
  targets.set(id, randomTarget())
  pushSnapshotFromTargets()
}

function removeBall() {
  const id = ballIds.value.pop()
  if (!id) return
  targets.delete(id)
  smoothed.delete(id)
  pushSnapshotFromTargets()
}

let tickTimer: ReturnType<typeof setInterval> | null = null

function startTicking() {
  tickTimer = setInterval(() => {
    tickCount.value++
    for (const id of ballIds.value) targets.set(id, randomTarget())
    pushSnapshotFromTargets()
  }, 1200)
}

function toggleAnimating() {
  if (isAnimating.value) {
    canvas.stopAnimating()
    isAnimating.value = false
    return
  }

  for (const ball of buffer.current.balls) {
    smoothed.set(ball.id, { x: ball.x, y: ball.y })
  }

  canvas.startAnimating(({ dt }) => {
    const lerpFactor = Math.min(1, dt / 150)
    const interpolated: Ball[] = ballIds.value.map((id) => {
      const target = targets.get(id)!
      const current = smoothed.get(id) ?? { x: target.x, y: target.y }
      current.x += (target.x - current.x) * lerpFactor
      current.y += (target.y - current.y) * lerpFactor
      smoothed.set(id, current)
      return { id, x: current.x, y: current.y, color: target.color }
    })
    buffer.push({ balls: interpolated })
  })
  isAnimating.value = true
}

onMounted(() => {
  addBall()
  addBall()
  addBall()
  startTicking()
})

onUnmounted(() => {
  if (tickTimer) clearInterval(tickTimer)
})
</script>

<template>
  <div class="w-screen h-screen bg-gray-900 text-white p-4 flex flex-col gap-4">
    <h1 class="text-xl font-bold">PixiJS composable smoke test (dev only)</h1>

    <div class="flex gap-4 flex-wrap items-center">
      <button class="px-3 py-1 bg-blue-600 rounded" @click="addBall">Add ball</button>
      <button class="px-3 py-1 bg-blue-600 rounded" @click="removeBall">Remove ball</button>
      <button class="px-3 py-1 bg-purple-600 rounded" @click="toggleAnimating">
        {{ isAnimating ? 'Stop' : 'Start' }} smooth interpolation
      </button>
    </div>

    <div class="flex gap-6 flex-wrap text-sm font-mono">
      <div>balls: {{ ballIds.length }}</div>
      <div>network ticks: {{ tickCount }}</div>
      <div>total renders: {{ renderCount }}</div>
      <div :class="rendersSinceLastTick > 1 ? 'text-yellow-400' : 'text-green-400'">
        renders since last tick: {{ rendersSinceLastTick }}
      </div>
      <div :class="canvasElementCount === 1 ? 'text-green-400' : 'text-red-400'">
        canvas elements in DOM: {{ canvasElementCount }}
      </div>
    </div>

    <p class="text-xs text-gray-400 max-w-2xl">
      Idle check: with interpolation OFF, "renders since last tick" should sit at 1 between ticks —
      it only climbs while interpolation is ON. Entity-count check: add/remove balls and confirm
      "canvas elements in DOM" stays at 1 and the other balls never flash/reset.
    </p>

    <div ref="containerRef" class="flex-1 rounded overflow-hidden"></div>
  </div>
</template>
