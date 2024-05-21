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
