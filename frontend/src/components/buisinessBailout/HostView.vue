<script setup lang="ts">
import { ref, defineProps, toRefs, computed } from 'vue'
import TimeComponent from '../TimeComponent.vue'
import { Graphics, type IPointData } from 'pixi.js'
import { Application } from 'vue3-pixi'

const value = ref(0)
const time = ref(0)

const props = defineProps<{
  height: number
  width: number
}>()
const { height, width } = toRefs(props)

const points = ref<IPointData[]>([])
const bailedPlayers = ref<BailedPlayer[]>([])
export type BailedPlayer = {
  name: string
  time: number
}
export type HostPayload = {
  value: number
  time: number
  bailed_players: BailedPlayer[]
}

function parsePayload(payload: string): HostPayload {
  return JSON.parse(payload)
}

function createPointData(value: number, time: number): IPointData {
  return { x: time, y: value }
}

function update(payload: string) {
  const { value: newValue, time: newTime, bailed_players } = parsePayload(payload)

  if (newValue > value.value) value.value = newValue
  if (bailedPlayers.value.length != bailed_players.length) {
    bailedPlayers.value = bailed_players
  }

  time.value = newTime

  points.value.push(createPointData(newValue, newTime))
}

defineExpose({ update })

const yMarginBottom = 100

const yHeight = computed(() => height.value - yMarginBottom)

function interpPosition(position: IPointData): [number, number] {
  const xScale = width.value / time.value
  const yScale = yHeight.value / value.value
  const newX = position.x * xScale
  const newY = yHeight.value - position.y * yScale

  return [newX, newY]
}

function render(graphics: Graphics) {
  graphics.clear()

  graphics.lineStyle(10, 0x000000)
  graphics.moveTo(0, yHeight.value)
  points.value.forEach((point) => {
    graphics.lineTo(...interpPosition(point))
  })

  graphics.lineStyle(4, 0xff0000)
  bailedPlayers.value.forEach((bailedPlayer) => {
    // Draw a vertical line at the time the player bailed
    const [x, y] = interpPosition({ x: bailedPlayer.time, y: 0 })
    graphics.moveTo(x, y)
    graphics.lineTo(x, 0)

    // Draw a circle at the point where the player bailed
    graphics.beginFill(0xff0000)
    graphics.drawCircle(x, y, 10)
    graphics.endFill()

    // Draw a label with the player's name at the bottom
    graphics.beginFill(0x000000)
    graphics.endFill()
  })
}
</script>
<template>
  <div class="absolute h-full w-full">
    <Application :height :width background-color="white">
      <Graphics :x="0" :y="0" @render="render" />
      <Text
        v-for="bailedPlayer in bailedPlayers"
        :key="bailedPlayer.name"
        :x="interpPosition({ x: bailedPlayer.time, y: 0 })[0] + 10"
        :y="interpPosition({ x: bailedPlayer.time, y: 0 })[1]"
        :text="bailedPlayer.name"
        :rotation="Math.PI * 0.25"
      />
    </Application>
  </div>
  <div class="flex m-4">
    <div>
      <TimeComponent :time-left="time" />
    </div>
    <h1>{{ value }}</h1>
  </div>
</template>
