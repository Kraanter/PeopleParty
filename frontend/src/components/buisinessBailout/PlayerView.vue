<script setup lang="ts">
import { ref } from 'vue'
type playerPayload = {
  value: number
}

const value = ref(0)

function parsePayload(payload: string): playerPayload {
  return JSON.parse(payload)
}

function update(payload: string) {
  const { value: newValue } = parsePayload(payload)

  value.value = newValue
}

defineExpose({ update })
</script>
<template>
  <div>
    <h1>{{ value }}</h1>
  </div>
</template>
