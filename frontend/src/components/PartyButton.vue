<script setup lang="ts">
import { defineEmits, defineProps } from 'vue'

defineProps<{
  disabled?: boolean
}>()

const emits = defineEmits(['click'])
</script>
<template>
  <button
    :disabled="disabled"
    @click="disabled ? null : emits('click')"
    class="bg-primary text-4xl text-white font-bold rounded-2xl w-full h-full disabled:bg-slate-400"
  >
    <slot></slot>
  </button>
</template>
