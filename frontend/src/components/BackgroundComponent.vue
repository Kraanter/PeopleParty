<script setup lang="ts">
import { type ObservablePoint, TilingSprite } from 'pixi.js'
import { Application, onTick } from 'vue3-pixi'
import { ref } from 'vue'

const color = 0xa0c8ff

let bgTexture = '/drawing.png'
let bgPos = ref({ x: 0, y: 0 })
const bgScale: ObservablePoint = { x: 0.3, y: 0.3 }

onTick(() => {
  bgPos.value.x += 0.5
})

let width = ref(window.innerWidth)
let height = ref(window.innerHeight)

// Listen for window resize events
window.addEventListener('resize', resize)

// Resize function window
function resize() {
  // Resize the renderer
  width.value = window.innerWidth
  height.value = window.innerHeight
}
</script>

<template>
  <Application :backgroundColor="0x6792d3">
    <tiling-sprite
      :width="width"
      :height="height"
      :texture="bgTexture"
      :tile-scale="bgScale"
      :tint="color"
      :tile-position="bgPos"
    >
    </tiling-sprite>
  </Application>
</template>

<style scoped></style>
