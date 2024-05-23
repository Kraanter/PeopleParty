<script setup lang="ts">
import { formatMilliseconds } from '@/util/funcs'
import { NCard } from 'naive-ui'
import { computed, defineProps, toRefs } from 'vue'

const props = defineProps<{
  timeLeft: number
}>()
const { timeLeft } = toRefs(props)

const timeIsLow = computed(() => timeLeft.value < 15 * 1000)
</script>
<template>
  <n-card :theme-overrides="{ paddingMedium: '1em' }">
    <div class="flex items-center">
      <img
        src="/assets/clock.svg"
        class="size-24 -m-8 mr-2"
        :class="{
          'animate-wiggle-fast': timeIsLow,
          'animate-wiggle-slow': !timeIsLow
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
