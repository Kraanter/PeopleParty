<!-- eslint-disable vue/multi-word-component-names -->
<script setup lang="ts">
import { NScrollbar, NCard } from 'naive-ui'
import { defineProps, toRefs, computed } from 'vue'
import { type Leaderboard } from '@/stores/confettiStore'
import PeoplePartyHeader from '@/components/PeoplePartyHeader.vue'
import TimeComponent from '../TimeComponent.vue'

const props = defineProps<{
  data: Leaderboard
}>()

const { data: leaderboard } = toRefs(props)

const sortedLeaderboard = computed(() => {
  const players = leaderboard?.value?.players ?? []
  return players.sort((a, b) => b.score - a.score)
})

const timeLeft = computed(() => {
  const time = leaderboard?.value?.time_left ?? 0
  return time
})

const pr = new Intl.PluralRules('en-US', { type: 'ordinal' })

const suffixes = new Map([
  ['one', 'st'],
  ['two', 'nd'],
  ['few', 'rd'],
  ['other', 'th']
])

const formatOrdinals = (n: number) => {
  const rule = pr.select(n)
  const suffix = suffixes.get(rule)
  return `${n}${suffix}`
}
</script>
<template>
  <div class="flex flex-col gap-4 w-full h-full">
    <PeoplePartyHeader />
    <!-- TODO: Make this justify-between and uncomment the skip component -->
    <div class="w-full flex justify-center px-8">
      <div>
        <TimeComponent :timeLeft="timeLeft" />
      </div>
      <!-- <div>
        <PartyButton class="w-24" @click="onClick">
          <svg
            xmlns="http://www.w3.org/2000/svg"
            viewBox="0 0 24 24"
            fill="currentColor"
            class="size-12 text-white mx-auto"
          >
            <path
              d="M5.055 7.06C3.805 6.347 2.25 7.25 2.25 8.69v8.122c0 1.44 1.555 2.343 2.805 1.628L12 14.471v2.34c0 1.44 1.555 2.343 2.805 1.628l7.108-4.061c1.26-.72 1.26-2.536 0-3.256l-7.108-4.061C13.555 6.346 12 7.249 12 8.689v2.34L5.055 7.061Z"
            /></svg
        ></PartyButton>
      </div> -->
    </div>
    <n-scrollbar class="-mb-4">
      <div class="mx-auto mb-4 w-4/5" v-for="(player, i) in sortedLeaderboard" :key="i">
        <n-card>
          <div class="w-full inline-flex justify-between text-2xl font-thin px-4">
            <p class="inline-flex">
              <span class="w-20">{{ formatOrdinals(i + 1) }}.</span
              ><span class="font-bold ml-4 col-span-5">{{ player.name }}</span>
            </p>
            <p>
              <span class="font-bold">{{ player.score }}</span> Points
            </p>
          </div>
        </n-card>
      </div>
    </n-scrollbar>
  </div>
</template>
