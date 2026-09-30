<script setup lang="ts">
import { computed, ref } from 'vue'
import { Graphics, Sprite, Text, type PointData } from 'pixi.js'
import { useGameCanvas, createEntityLayer, loadTexture } from '@/composables/pixi'
import { useColorStore } from '@/stores/colorStore'
import { unLerp } from '@/util/funcs'
import { deCasteljau } from '../spline'
import type { BailedPlayer } from '../parser'

const props = defineProps<{
  points: PointData[]
  bailedPlayers: BailedPlayer[]
}>()

const colorStore = useColorStore()

// Same two-independent-axes chart as the live view — no aspect ratio to preserve, so this
// deliberately omits worldSize/size and trusts the container's own measured box.
const xMargin = 10
const ROCKET_SIZE = 75
const ROCKET_TEXTURE = '/assets/games/businessBailout/rocket.svg'

// points/bailedPlayers are set once when the Results screen mounts and never change again
// (HostView freezes `points` the moment it flips to the Results state) — no snapshot buffer
// needed, drawScene() can just read the props directly.

// 9-second auto-pan replay reveal, driven by real per-frame dt off the shared rAF loop instead
// of the original's own setInterval(10ms) + Date.now() timer.
const pageTime = 9 * 1000
let pageCurTime = 0

const containerRef = ref<HTMLElement | null>(null)
const gfx = new Graphics()

const canvas = useGameCanvas({
  container: containerRef,
  render: () => drawScene()
})
canvas.root.addChild(gfx)

const rocket = new Sprite()
rocket.anchor.set(0.5, 0)
rocket.width = ROCKET_SIZE
rocket.height = ROCKET_SIZE * 2
rocket.rotation = Math.PI
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

const endTime = computed(() => props.points[props.points.length - 1].x)
const maxValue = computed(() => Math.max(...props.points.map((point) => point.y)))
const xWidth = computed(() => canvas.width.value - xMargin * 2)
const yHeight = computed(() => canvas.height.value - 10)

function curPointPercent(): number {
  return unLerp(-(pageTime / 5), pageTime, Math.min(pageCurTime, pageTime))
}

function interpPosition(point: PointData): [number, number] {
  const percent = curPointPercent()
  const focusPointIndex = percent * (props.points.length - 1)

  const controlPoints = props.points.slice(
    Math.max(0, Math.floor(focusPointIndex) - 25),
    Math.min(props.points.length, Math.floor(focusPointIndex) + 25)
  )
  const focusPoint = deCasteljau(controlPoints, percent)

  const viewPortWidth = (endTime.value / 10) * 2
  const viewPortHeight = (maxValue.value / 10) * 2

  const x =
    unLerp(focusPoint.x - viewPortWidth, focusPoint.x + viewPortWidth, point.x) * xWidth.value +
    xMargin
  const y =
    unLerp(focusPoint.y - viewPortHeight, focusPoint.y + viewPortHeight, point.y) * yHeight.value

  return [x, yHeight.value - y]
}

function drawScene() {
  if (props.points.length === 0) return

  gfx.clear()
  gfx.moveTo(...interpPosition(props.points[0]))
  for (const point of props.points) gfx.lineTo(...interpPosition(point))
  gfx.stroke({ width: 10, color: colorStore.colorPalette.primary.base.number })

  if (props.bailedPlayers.length > 0) {
    for (const bailedPlayer of props.bailedPlayers) {
      const [x, y] = interpPosition({ x: bailedPlayer.time, y: bailedPlayer.value })
      gfx.circle(x, y, 15)
    }
    gfx.fill({ color: 0xff0000 })
    gfx.stroke({ width: 4, color: 0xff0000 })
  }

  labelLayer.sync(props.bailedPlayers)

  const [rx, ry] = interpPosition(props.points[props.points.length - 1])
  rocket.position.set(rx, ry)
}

canvas.startAnimating(({ dt }) => {
  pageCurTime = Math.min(pageCurTime + dt, pageTime)
  if (pageCurTime >= pageTime) canvas.stopAnimating()
})
</script>

<template>
  <div ref="containerRef" class="w-full h-full"></div>
</template>
