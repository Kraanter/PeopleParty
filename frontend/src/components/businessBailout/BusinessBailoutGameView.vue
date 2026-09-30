<script setup lang="ts">
import { computed, ref, watch } from 'vue'
import { Graphics, Sprite, Text, type PointData } from 'pixi.js'
import {
  useGameCanvas,
  createEntityLayer,
  useSnapshotBuffer,
  loadTexture
} from '@/composables/pixi'
import { useColorStore } from '@/stores/colorStore'
import type { BailedPlayer, BusinessBailoutHostData } from './parser'

const props = defineProps<{ hostData: BusinessBailoutHostData }>()

const colorStore = useColorStore()

// Two independent axes (value vs. time) — a chart has no single "aspect ratio" to preserve, and
// the bail-marker circle is drawn at a fixed pixel radius in already-mapped pixel space, so
// nothing here can visually warp. Deliberately omits worldSize/size and just trusts the
// container's own measured box, same as the raw width/height props the pre-migration code used.
const xMargin = 25
const yMargin = 25
const ROCKET_SIZE = 75
const ROCKET_TEXTURE = '/assets/games/businessBailout/rocket.svg'

const containerRef = ref<HTMLElement | null>(null)
const gfx = new Graphics()

const canvas = useGameCanvas({
  container: containerRef,
  backgroundColor: 0x000000,
  backgroundAlpha: 1,
  render: () => drawScene()
})
canvas.root.addChild(gfx)

const rocket = new Sprite()
rocket.anchor.set(0.5, 0)
rocket.width = ROCKET_SIZE
rocket.height = ROCKET_SIZE * 2
canvas.root.addChild(rocket)
loadTexture(rocket, ROCKET_TEXTURE, canvas.invalidate)

const labelLayer = createEntityLayer<BailedPlayer, Text>(canvas.root, {
  key: (player) => player.name,
  create: () => {
    const text = new Text({ text: '', style: { fill: 0xffffff } })
    text.anchor.set(1.1, 0.07)
    text.rotation = Math.PI * 0.25
    return text
  },
  update: (text, player) => {
    text.text = player.name
    const [x, y] = interpPosition({ x: player.time, y: player.value })
    text.position.set(x + 10, y)
  }
})

const buffer = useSnapshotBuffer<BusinessBailoutHostData>(props.hostData, {
  invalidate: canvas.invalidate
})

const xWidth = computed(() => canvas.width.value - xMargin * 2)
const yHeight = computed(() => canvas.height.value - yMargin * 2)

// Reassigned at the top of every drawScene() call (current points' time/value range changes as
// the chart grows) — labelLayer's update() callback above reads whichever version is current
// at sync() time, since sync() only ever runs from inside drawScene() itself.
let interpPosition: (point: PointData) => [number, number] = () => [0, 0]

function drawScene() {
  const { points, bailed_players } = buffer.current
  if (points.length === 0) return

  const timeNow = points[points.length - 1].x
  const maxValue = Math.max(...points.map((point) => point.y))
  interpPosition = (point) => {
    const xStep = xWidth.value / timeNow
    const yStep = yHeight.value / maxValue
    const newX = point.x * xStep + xMargin
    const newY = canvas.height.value - (point.y * yStep + yMargin)
    return [newX, newY]
  }

  gfx.clear()

  // Price line
  gfx.moveTo(...interpPosition(points[0]))
  for (const point of points) gfx.lineTo(...interpPosition(point))
  gfx.stroke({ width: 10, color: colorStore.colorPalette.primary.base.number })

  // X axis
  gfx.moveTo(...interpPosition({ x: 0, y: 0 }))
  for (let timeIncrement = 0; timeIncrement < timeNow; timeIncrement += 2000) {
    const position = interpPosition({ x: timeIncrement, y: 0 })
    gfx.lineTo(...position)
    gfx.lineTo(position[0], position[1] + 10)
    gfx.lineTo(position[0], position[1] - 10)
    gfx.lineTo(...position)
  }
  gfx.lineTo(...interpPosition({ x: timeNow, y: 0 }))
  gfx.stroke({ width: 4, color: 0xffffff })

  // Bailed-player markers (vertical drop line + circle)
  if (bailed_players.length > 0) {
    for (const bailedPlayer of bailed_players) {
      const [x, y] = interpPosition({ x: bailedPlayer.time, y: bailedPlayer.value })
      gfx.moveTo(x, interpPosition({ x: 0, y: 0 })[1])
      gfx.lineTo(x, y)
      gfx.circle(x, y, 15)
    }
    gfx.fill({ color: 0xff0000 }) // no-op on the open drop lines, fills the marker circles
    gfx.stroke({ width: 4, color: 0xff0000 })
  }

  labelLayer.sync(bailed_players)

  const last = points[points.length - 1]
  const [rx, ry] = interpPosition(last)
  rocket.position.set(rx, ry)

  let angle = 0
  if (points.length >= 2) {
    const lookbackSteps = Math.min(5, points.length - 1)
    const earlier = points[points.length - 1 - lookbackSteps]
    const [ex, ey] = interpPosition(earlier)
    angle = Math.atan2(rx - ex, ry - ey)
  }
  rocket.rotation = Math.PI - angle
}

watch(
  () => props.hostData,
  (data) => buffer.push(data),
  { immediate: true }
)
</script>

<template>
  <div ref="containerRef" class="w-full h-full"></div>
</template>
