<script setup lang="ts">
import { type ObservablePoint, TilingSprite } from 'pixi.js'
import { Application, onTick } from 'vue3-pixi'
import { ref, watchEffect } from 'vue'
import { useColorStore } from '@/stores/colorStore';
import { storeToRefs } from 'pinia';

const colorStore = useColorStore();

watchEffect(() => {
  // Set background color variables in css
  document.documentElement.style.setProperty('--primary', colorStore.colorPalette.primary.string);
  document.documentElement.style.setProperty('--secondary', colorStore.colorPalette.secondary.string);
})

const { colorPalette } = storeToRefs(colorStore);

let bgTexture = '/drawing.png'
let bgPos = ref({ x: 0, y: 0 })
// @ts-ignore
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
  <Application :width :height :background-alpha="0">
    <tiling-sprite :width :height :texture="bgTexture" :tile-scale="bgScale" :tint="colorPalette.primary.number"
      :tile-position="bgPos">
    </tiling-sprite>
  </Application>
</template>