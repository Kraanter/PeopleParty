<script setup lang="ts">
import { formatMilliseconds } from '@/util/funcs'
import { NCard } from 'naive-ui'
import { computed, defineProps, ref, toRefs, watch } from 'vue'

const props = defineProps<{
  timeLeft: number
}>()
const { timeLeft } = toRefs(props)

let previousTime = timeLeft.value
const recentlyChanged = ref(true);

watch(timeLeft, (newTime: number) => {
  if (newTime !== previousTime) {
    previousTime = newTime
    recentlyChanged.value = true
  }

  setTimeout(() => {
    recentlyChanged.value = false
  }, 1000)
})


const timeIsLow = computed(() => timeLeft.value < 15 * 1000)
</script>
<template>
  <n-card :theme-overrides="{ paddingMedium: '1em' }">
    <div class="flex items-center">
      <img
        src="/assets/clock.svg"
        class="size-24 -m-8 mr-2"
        :class="{
          'animate-wiggle-fast': timeIsLow && recentlyChanged,
          'animate-wiggle-slow': !timeIsLow && recentlyChanged
        }"
      />
      <span
        class="text-4xl font-bold"
        :class="{ 'animate-pulse': timeLeft < 5 * 1000, 'text-primary': timeIsLow }"
      >
        {{ formatMilliseconds(timeLeft) }}</span
      >
    </div>
  </n-card>
</template>
