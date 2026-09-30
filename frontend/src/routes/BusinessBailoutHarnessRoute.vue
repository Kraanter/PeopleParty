<script setup lang="ts">
import { ref } from 'vue'
import HostView from '@/components/businessBailout/HostView.vue'
import type { BailedPlayer } from '@/components/businessBailout/parser'

const hostViewRef = ref<InstanceType<typeof HostView>>()

let time = 0
let value = 100
let bailed: BailedPlayer[] = []
let tickTimer: ReturnType<typeof setInterval> | null = null

function push() {
  hostViewRef.value?.__devUpdateHost?.(value, time, bailed)
}

function start() {
  time = 0
  value = 100
  bailed = []
  push()
  tickTimer = setInterval(() => {
    time += 200
    value += (Math.random() - 0.3) * 40
    if (value < 10) value = 10
    push()
  }, 50)
}

function stop() {
  if (tickTimer) clearInterval(tickTimer)
  tickTimer = null
}

function bailPlayer(name: string) {
  bailed = [...bailed, { name, time, value }]
  push()
}

function showResults() {
  hostViewRef.value?.__devUpdateResults?.(bailed)
}
</script>

<template>
  <div class="w-screen h-screen bg-gray-900 text-white p-4 flex flex-col gap-4">
    <div class="flex gap-2 relative z-50">
      <button class="px-3 py-1 bg-blue-600 rounded" @click="start">Start</button>
      <button class="px-3 py-1 bg-gray-600 rounded" @click="stop">Stop</button>
      <button class="px-3 py-1 bg-yellow-600 rounded" @click="bailPlayer('Player 1')">
        Bail Player 1
      </button>
      <button class="px-3 py-1 bg-orange-600 rounded" @click="bailPlayer('Player 2')">
        Bail Player 2
      </button>
      <button class="px-3 py-1 bg-purple-600 rounded" @click="showResults">Show Results</button>
    </div>
    <div id="harness-mount" class="flex-1 border border-gray-700 relative">
      <HostView ref="hostViewRef" :width="1000" :height="700" />
    </div>
  </div>
</template>
