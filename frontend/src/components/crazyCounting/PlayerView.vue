<script setup lang="ts">
import { defineProps, toRefs } from 'vue'

const props = defineProps<{
  width: number
  height: number
  data: { amount: number; time: number; submitted: boolean }
}>()

const { data } = toRefs(props)

// TODO: Implement communication with confetti
const submitAnswer = () => {
  console.log('Locking in answer')
}

const increment = () => {
  data.value.amount++
}

const decrement = () => {
  data.value.amount--
}
</script>
<template>
  <div class="w-full h-full flex">
    <div class="grid grid-rows-4 text-center my-auto max-w-full h-full gap-y-12 max-h-[75%] w-full">
      <div class="my-auto">
        <span class="text-8xl font-bold">{{ data.amount }}</span>
      </div>
      <div class="grid grid-cols-2 row-span-2 w-full h-full rounded-3xl overflow-hidden">
        <button
          class="active:bg-opacity-100 bg-opacity-50 bg-red-500 border-r-2 border-dashed"
          @click="decrement"
          :disabled="data.amount <= 0"
        >
          <span class="text-8xl font-bold">-</span>
        </button>
        <button
          class="active:bg-opacity-100 bg-opacity-50 bg-green-500 border-l-2 border-dashed"
          @click="increment"
        >
          <span class="text-8xl font-bold">+</span>
        </button>
      </div>
      <button
        class="bg-red-400 active:bg-red-500 rounded-full text-white grid p-4"
        @click="submitAnswer"
      >
        <svg
          xmlns="http://www.w3.org/2000/svg"
          fill="none"
          viewBox="0 0 24 24"
          stroke-width="1.5"
          stroke="currentColor"
          class="w-full h-full"
        >
          <path
            stroke-linecap="round"
            stroke-linejoin="round"
            d="M16.5 10.5V6.75a4.5 4.5 0 1 0-9 0v3.75m-.75 11.25h10.5a2.25 2.25 0 0 0 2.25-2.25v-6.75a2.25 2.25 0 0 0-2.25-2.25H6.75a2.25 2.25 0 0 0-2.25 2.25v6.75a2.25 2.25 0 0 0 2.25 2.25Z"
          />
        </svg>
        <span class="text-2xl font-bold">Press to lock in</span>
        <span class="text-lg">{{ data.time }} seconds left</span>
      </button>
    </div>
  </div>
</template>
