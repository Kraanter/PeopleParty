<!-- eslint-disable vue/multi-word-component-names -->
<script setup lang="ts">
import { formatMilliseconds } from '@/util/funcs'
import { NScrollbar, NCard } from 'naive-ui'
import { defineProps, toRefs, computed } from 'vue'
import { type Leaderboard } from '@/stores/confettiStore'
import PeoplePartyHeader from '@/components/PeoplePartyHeader.vue'
import PartyButton from '../PartyButton.vue'

const props = defineProps<{
  data: Leaderboard
}>()

const { data: leaderboard } = toRefs(props)

const sortedLeaderboard = computed(() => {
  const players = leaderboard.value.players
  return players.sort((a, b) => b.score - a.score)
})

const pr = new Intl.PluralRules('en-US', { type: 'ordinal' })

const suffixes = new Map([
  ['one', 'st'],
  ['two', 'nd'],
  ['few', 'rd'],
  ['other', 'th']
])
const formatOrdinals = (n) => {
  const rule = pr.select(n)
  const suffix = suffixes.get(rule)
  return `${n}${suffix}`
}

const onClick = () => console.log('TEST')
</script>
<template>
  <div class="flex flex-col gap-4 w-full h-full">
    <PeoplePartyHeader />
    <!-- TODO: Make this justify-between and uncomment the skip component -->
    <div class="w-full flex justify-center px-8">
      <div>
        <n-card :theme-overrides="{ paddingMedium: '1em' }">
          <div class="flex items-center">
            <svg
              xmlns="http://www.w3.org/2000/svg"
              viewBox="0 0 24 24"
              fill="currentColor"
              class="size-16 -m-4 mr-2"
            >
              <path
                fill-rule="evenodd"
                d="M12 2.25c-5.385 0-9.75 4.365-9.75 9.75s4.365 9.75 9.75 9.75 9.75-4.365 9.75-9.75S17.385 2.25 12 2.25ZM12.75 6a.75.75 0 0 0-1.5 0v6c0 .414.336.75.75.75h4.5a.75.75 0 0 0 0-1.5h-3.75V6Z"
                clip-rule="evenodd"
              />
            </svg>
            <span class="text-2xl font-bold"> {{ formatMilliseconds(data.time_left) }}</span>
          </div>
        </n-card>
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
