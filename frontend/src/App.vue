<script setup lang="ts">
import { RouterView } from 'vue-router'
import { ref } from 'vue'
import Background from '@/components/BackgroundComponent.vue'

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
  <main class="w-full h-full">
    <RouterView class="absolute z-10 max-w-80 max-h-80" />

    <Background :width="width" :height="height" class="fixed top-0 left-0"></Background>
  </main>
</template>
