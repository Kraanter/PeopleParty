<script setup lang="ts">
import { ref } from 'vue'
import TimeComponent from '../TimeComponent.vue'

const value = ref(0)
const time = ref(0)

type hostPayload = {
  value: number
  time: number
}

function parsePayload(payload: string): hostPayload {
  return JSON.parse(payload)
}

function update(payload: string) {
  const { value: newValue, time: newTime } = parsePayload(payload)

  console.log('Updating: value,', value, newValue)
  console.log('Updating: time,', time, newTime)

  value.value = newValue
  time.value = newTime
}

defineExpose({ update })
</script>
<template>
  <div>
    <TimeComponent :time-left="time" />
    <h1>{{ value }}</h1>
  </div>
</template>
