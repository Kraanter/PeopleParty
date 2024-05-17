<script setup lang="ts">
import { defineEmits } from 'vue'

const emits = defineEmits(['click'])
</script>
<template>
  <button
    @click="emits('click')"
    class="bg-primary text-4xl text-white font-bold px-12 py-6 rounded-2xl"
  >
    <slot></slot>
  </button>
</template>
<style>
button {
  box-shadow: 0.5rem 0.5rem 0 #000;
  transition:
    transform 0.1s ease-in-out,
    box-shadow 0.1s ease-in-out;
}

button:active {
  transform: translate(0.5rem, 0.5rem);
  box-shadow: 0.1rem 0.1rem 0 #000;
}
</style>
