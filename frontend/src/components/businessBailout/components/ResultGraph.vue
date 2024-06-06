<script setup lang="ts">
import { Application } from 'vue3-pixi'
import { computed, defineProps, ref, onMounted, toRefs } from 'vue'
import type { IPointData, Graphics } from 'pixi.js'
import { useColorStore } from '@/stores/colorStore'
import { unLerp } from '@/util/funcs'
import { deCasteljau } from '../spline'
import type { BailedPlayer } from '../parser'

const props = defineProps<{
  width: number
  height: number
  points: IPointData[]
  bailedPlayers: BailedPlayer[]
}>()

const colorStore = useColorStore()

const { width, height, points, bailedPlayers } = toRefs(props)

// Get the unix timestamp of the start of the result page
const pageStartTime = Date.now()
const pageTime = 9 * 1000
const pageCurTime = ref(Date.now() - pageStartTime)

onMounted(() => {
  const interval = setInterval(() => {
    pageCurTime.value = Date.now() - pageStartTime
  }, 10)

  return () => clearInterval(interval)
})

const endTime = computed(() => points.value[points.value.length - 1].x)
const maxValue = computed(() => Math.max(...points.value.map((point) => point.y)))

const curPointPercent = computed(() =>
  unLerp(-(pageTime / 5), pageTime, Math.min(pageCurTime.value, pageTime))
)

const xMargin = 10
const xWidth = computed(() => width.value - xMargin * 2)
const yHeight = computed(() => height.value - 10)

function interpPosition(point: IPointData): [number, number] {
  const focusPointIndex = curPointPercent.value * (points.value.length - 1)

  const controlPoints = points.value.slice(
    Math.max(0, Math.floor(focusPointIndex) - 25),
    Math.min(points.value.length, Math.floor(focusPointIndex) + 25)
  )
  const focusPoint = deCasteljau(controlPoints, curPointPercent.value)

  const viewPortWidth = (endTime.value / 10) * 2
  const viewPortHeight = (maxValue.value / 10) * 2

  const x =
    unLerp(focusPoint.x - viewPortWidth, focusPoint.x + viewPortWidth, point.x) * xWidth.value +
    xMargin
  const y =
    unLerp(focusPoint.y - viewPortHeight, focusPoint.y + viewPortHeight, point.y) * yHeight.value

  return [x, yHeight.value - y]
}

function render(graphics: Graphics) {
  graphics.clear()

  graphics.lineStyle(10, colorStore.colorPalette.primary.base.number)
  graphics.moveTo(...interpPosition(points.value[0]))
  points.value.forEach((point) => {
    graphics.lineTo(...interpPosition(point))
  })

  graphics.lineStyle(4, 0xff0000)
  bailedPlayers.value.forEach((bailedPlayer) => {
    // Draw a vertical line at the time the player bailed
    const [x, y] = interpPosition({ x: bailedPlayer.time, y: bailedPlayer.value })
    // Draw a circle at the point where the player bailed
    graphics.beginFill(0xff0000)
    graphics.drawCircle(x, y, 15)
    graphics.endFill()
  })
}
</script>
<template>
  <Application :height :width backgroud-color="black">
    <Graphics :x="0" :y="0" @render="render" />
    <Text
      v-for="bailedPlayer in bailedPlayers"
      :key="bailedPlayer.name"
      :style="{ fill: 'white' }"
      :anchor-x="1.1"
      :anchor-y="0.07"
      :x="interpPosition({ x: bailedPlayer.time, y: bailedPlayer.value })[0] + 10"
      :y="interpPosition({ x: bailedPlayer.time, y: bailedPlayer.value })[1]"
      :text="bailedPlayer.name"
      :rotation="Math.PI * 0.25"
    />
  </Application>
</template>
