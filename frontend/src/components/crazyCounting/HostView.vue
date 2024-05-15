<script lang="ts" setup>
import { Application, onTick, Loader } from 'vue3-pixi'
import { ref, onMounted, toRefs, defineProps } from 'vue'

type PosData = {
  x: number
  y: number
}

const size = ref(100)

const props = defineProps<{
  width: number,
  height: number,
  data: PosData[]
}>()

const { width, height, data } = toRefs(props)

const interpolatePosition = (pos: PosData) => {
  return {
    x: Math.abs(pos.x) * (width.value - size.value),
    y: Math.abs(pos.y) * (height.value - size.value)
  }
}

// TODO: Remove animation and connect to the server
onMounted(() => {
  onTick(() => {
    data.value.forEach((pos) => {
      let newPosX = pos.x + 0.01
      let newPosY = pos.y + 0.01

      if (newPosX > 1) {
        newPosX = -1
      }

      if (newPosY > 1) {
        newPosY = -1
      }

      pos.x = newPosX
      pos.y = newPosY
    })
  })
})
</script>
<template>
  <Loader :resources="['/circle.svg']">
    <template #fallback="{ progress }">
      <text :x="120" :y="120" :anchor="0.5">
        <!-- TODO: Add a nice loading screen -->
        {{ `Loading... ${progress}` }}
      </text>
    </template>
    <Application :width :height background-color="white">
      <sprite v-for="(pos, i) in data" :position="interpolatePosition(pos)" :key="i" texture="/circle.svg" />
    </Application>
  </Loader>
</template>