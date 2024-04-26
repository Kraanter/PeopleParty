<script setup lang="ts">
import { RouterView } from 'vue-router'
import Background from '@/components/BackgroundComponent.vue'
</script>

<template>
  <main class="relative w-screen min-h-screen h-screen">
    <RouterView class="absolute z-10 w-full h-full" />

    <Background class="absolute w-full h-full top-0 left-0"></Background>
  </main>
</template>
