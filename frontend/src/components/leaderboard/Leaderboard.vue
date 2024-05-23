<!-- eslint-disable vue/multi-word-component-names -->
<script setup lang="ts">
import { NList, NListItem } from 'naive-ui'
import { useWebSocketStore } from '@/stores/confettiStore'
import { storeToRefs } from 'pinia'

const confettiStore = useWebSocketStore()
const { leaderboard } = storeToRefs(confettiStore)

const emit = defineEmits(['click'])
</script>
<template>
  <div class="flex flex-col gap-4 w-full h-full">
    <div class="grid grid-cols-3 gap-12 grid-rows-1 mb-8 h-full w-full">
      <div class="w-full h-full flex flex-col justify-between">
        time left {{ leaderboard.time_left }}
        <n-list>
          <n-list-item v-for="(player, i) in leaderboard.players" :key="i">
            <p>{{ player.name }}</p>
            <p>{{ player.score }}</p>
          </n-list-item>
        </n-list>
      </div>
    </div>
  </div>
</template>
