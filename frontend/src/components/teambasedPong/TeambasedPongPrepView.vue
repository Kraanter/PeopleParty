<script lang="ts" setup>
import { ref } from 'vue'
import { Graphics } from 'pixi.js'
import { useGameCanvas } from '@/composables/pixi'

// Fully static preview — the same fixed field every round, no live data at all — so this draws
// once at setup and never needs to redraw again.
const WORLD_WIDTH = 800
const WORLD_HEIGHT = 600

const containerRef = ref<HTMLElement | null>(null)
const gfx = new Graphics()

const canvas = useGameCanvas({
  container: containerRef,
  worldSize: { width: WORLD_WIDTH, height: WORLD_HEIGHT },
  backgroundColor: 0x000000,
  backgroundAlpha: 1,
  render: () => drawScene()
})
canvas.root.addChild(gfx)

function drawScene() {
  gfx.clear()

  // Center line
  for (let y = 0; y < WORLD_HEIGHT; y += 30) {
    gfx.moveTo(WORLD_WIDTH / 2, y)
    gfx.lineTo(WORLD_WIDTH / 2, y + 15)
  }
  gfx.stroke({ width: 3, color: 0xffffff, alpha: 0.5 })

  // Paddles at rest, centered
  gfx.rect(30 - 10, WORLD_HEIGHT / 2 - 60, 20, 120).fill(0x00ff00)
  gfx.rect(WORLD_WIDTH - 30 - 10, WORLD_HEIGHT / 2 - 60, 20, 120).fill(0xff0000)

  // Ball at rest, centered
  gfx.circle(WORLD_WIDTH / 2, WORLD_HEIGHT / 2, 10).fill(0xffffff)
}
</script>

<template>
  <div ref="containerRef" class="w-full h-full flex items-center justify-center"></div>
</template>
