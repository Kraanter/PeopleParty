<script lang="ts" setup>
import { ref, watch } from 'vue'
import { Graphics, Text, TextStyle, CanvasTextMetrics } from 'pixi.js'
import { useGameCanvas } from '@/composables/pixi'
import type { MarbleManiaResult } from './MarbleManiaModels'

const props = defineProps<{
  results: MarbleManiaResult | null
  getPlayerColor: (playerId: string) => number
}>()

const VIEWPORT_WIDTH = 800
const VIEWPORT_HEIGHT = 600

const nameStyle = new TextStyle({
  fontFamily: ['Helvetica', 'Arial', 'sans-serif'],
  fontSize: 24,
  fill: 'white',
  stroke: { color: 'black', width: 3 }
})
const nameStyleUnfinished = new TextStyle({ ...nameStyle, fill: 'gray' })
const timeStyle = new TextStyle({
  fontFamily: ['Helvetica', 'Arial', 'sans-serif'],
  fontSize: 16,
  fill: 'lightblue',
  stroke: { color: 'black', width: 2 }
})
const dnfStyle = new TextStyle({ ...timeStyle, fill: 'red' })

const pr = new Intl.PluralRules('en-US', { type: 'ordinal' })
const suffixes = new Map([
  ['one', 'st'],
  ['two', 'nd'],
  ['few', 'rd'],
  ['other', 'th']
])
const formatOrdinals = (n: number) => `${n}${suffixes.get(pr.select(n))}`

function darken(color: number): number {
  return (
    Math.floor((color & 0xff) * 0.6) |
    (Math.floor(((color >> 8) & 0xff) * 0.6) << 8) |
    (Math.floor(((color >> 16) & 0xff) * 0.6) << 16)
  )
}

function lighten(color: number): number {
  return (
    Math.min(255, Math.floor((color & 0xff) * 1.3)) |
    (Math.min(255, Math.floor(((color >> 8) & 0xff) * 1.3)) << 8) |
    (Math.min(255, Math.floor(((color >> 16) & 0xff) * 1.3)) << 16)
  )
}

function getCenteredTextPosition(text: string, x: number, style: TextStyle): number {
  const metrics = CanvasTextMetrics.measureText(text, style)
  return x - metrics.width / 2
}

const containerRef = ref<HTMLElement | null>(null)

// Fully static — built once when results arrive, never redrawn afterward.
const canvas = useGameCanvas({
  container: containerRef,
  worldSize: { width: VIEWPORT_WIDTH, height: VIEWPORT_HEIGHT },
  backgroundColor: 0x000000,
  render: () => {}
})

function buildScene(result: MarbleManiaResult) {
  const bg = new Graphics()
  bg.rect(0, 0, VIEWPORT_WIDTH, VIEWPORT_HEIGHT).fill({ color: 0x1a1a1a, alpha: 0.8 }).stroke({
    width: 2,
    color: 0x333333
  })
  canvas.root.addChild(bg)

  result.results.forEach((r, idx) => {
    const marbleX = VIEWPORT_WIDTH / 2 - 180
    const marbleY = 50 + idx * 60
    const marbleRadius = 16
    const color = props.getPlayerColor(r.name)
    const border = darken(color)
    const highlight = lighten(color)

    const marbleGfx = new Graphics()
    marbleGfx.circle(marbleX, marbleY, marbleRadius).fill(color).stroke({ width: 2, color: border })
    marbleGfx
      .circle(marbleX - marbleRadius * 0.3, marbleY - marbleRadius * 0.3, marbleRadius * 0.3)
      .fill({ color: highlight, alpha: 0.6 })
    canvas.root.addChild(marbleGfx)

    const nameText = new Text({
      text: `${formatOrdinals(r.placement)}. ${r.name}`,
      style: r.has_finished ? nameStyle : nameStyleUnfinished
    })
    nameText.position.set(150, 25 + idx * 60)
    canvas.root.addChild(nameText)

    if (r.has_finished) {
      const label = `Time: ${r.time_to_finish.toFixed(2)}s`
      const timeText = new Text({ text: label, style: timeStyle })
      timeText.position.set(
        getCenteredTextPosition(label, VIEWPORT_WIDTH / 2 - 100, timeStyle),
        50 + idx * 60
      )
      canvas.root.addChild(timeText)
    } else {
      const label = 'Did not finish'
      const dnfText = new Text({ text: label, style: dnfStyle })
      dnfText.position.set(
        getCenteredTextPosition(label, VIEWPORT_WIDTH / 2 - 100, dnfStyle),
        50 + idx * 60
      )
      canvas.root.addChild(dnfText)
    }
  })

  canvas.invalidate()
}

// A reactive prop, not a push()-via-ref call — results are sent exactly once, with no retry
// cadence to self-correct a dropped call the way high-frequency snapshots do.
watch(
  () => props.results,
  (result) => {
    if (result) buildScene(result)
  },
  { immediate: true }
)
</script>

<template>
  <div class="flex flex-col h-full w-full items-center">
    <div class="text-4xl m-6 text-white shrink-0">Marble Mania Results</div>
    <div class="relative flex-1 w-full min-h-0">
      <div class="absolute top-0 left-0 w-full h-full bg-black"></div>
      <div
        ref="containerRef"
        class="relative w-full h-full flex items-center justify-center overflow-hidden"
      ></div>
    </div>
  </div>
</template>
