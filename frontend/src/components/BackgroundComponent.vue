<script setup lang="ts">
import { type ObservablePoint } from 'pixi.js'
import { Application, onTick } from 'vue3-pixi'
import { ref, onMounted, onUnmounted } from 'vue'
import { useColorStore } from '@/stores/colorStore'
import { storeToRefs } from 'pinia'

const colorStore = useColorStore()
const { colorPalette } = storeToRefs(colorStore)

const bgTexture = '/drawing.png'

// @ts-ignore
const bgScale: ObservablePoint = { x: 0.3, y: 0.3 }

// Use non-reactive position that resets to prevent infinite growth
let xPosition = 0
const ANIMATION_SPEED = 0.5
const RESET_THRESHOLD = 512 * bgScale.x // multiple with the scale
// needs to be a multiple of 512 (image width)

// Create reactive position that updates efficiently
const bgPos = ref({ x: 0, y: 0 })

// Optimized animation loop
onTick(() => {
  // Increment internal position
  xPosition += ANIMATION_SPEED
  
  // Reset position periodically to prevent infinite growth
  if (xPosition >= RESET_THRESHOLD) {
    xPosition = 0
  }
  
  // Update reactive position (only when necessary)
  bgPos.value = { x: xPosition, y: 0 }
})

const width = ref(window.innerWidth)
const height = ref(window.innerHeight)

// Optimized resize function
const resize = () => {
  width.value = window.innerWidth
  height.value = window.innerHeight
}

// Properly manage event listeners
onMounted(() => {
  window.addEventListener('resize', resize, { passive: true })
})

onUnmounted(() => {
  window.removeEventListener('resize', resize)
  // Reset position on cleanup
  xPosition = 0
})
</script>

<template>
  <Application :width :height :background-alpha="0">
    <tiling-sprite
      :width
      :height
      :texture="bgTexture"
      :tile-scale="bgScale"
      :tint="colorPalette.secondary.dark.number"
      :tile-position="bgPos"
    >
    </tiling-sprite>
  </Application>
</template>
