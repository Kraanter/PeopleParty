<script setup lang="ts">
import { defineProps } from 'vue'

defineProps<{
  disabled?: boolean
}>()
</script>
<template>
  <button
    :disabled="disabled"
    class="bg-primary text-4xl text-white font-bold rounded-2xl w-full h-full disabled:bg-slate-400"
  >
    <slot></slot>
  </button>
</template>
