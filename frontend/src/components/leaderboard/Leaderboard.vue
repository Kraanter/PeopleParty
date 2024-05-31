<!-- eslint-disable vue/multi-word-component-names -->
<script setup lang="ts">
import { NScrollbar, NCard } from 'naive-ui'
import { defineProps, toRefs, computed, } from 'vue'
import { useWebSocketStore, type Leaderboard } from '@/stores/confettiStore'
import PeoplePartyHeader from '@/components/PeoplePartyHeader.vue'
import TimeComponent from '../TimeComponent.vue'
import PartyButton from '../PartyButton.vue'

import * as flatbuffers from 'flatbuffers'
import { LeaderboardHostSkipPayload,
  LeaderboardPayload,
  LeaderboardPayloadType,
  LeaderboardType,
  MessageType,
  Payload } from '@/flatbuffers/messageClass'
import { buildMessage } from '@/util/flatbufferMessageBuilder'

const props = defineProps<{
  data: Leaderboard
}>()

const websocketStore = useWebSocketStore()

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

const skipLeaderboard = () => {
  let builder = new flatbuffers.Builder()

  let leaderboardMessage = LeaderboardHostSkipPayload.createLeaderboardHostSkipPayload (
    builder,
    true
  )

  let message = LeaderboardPayloadType.createLeaderboardPayloadType(
    builder,
    LeaderboardType.LeaderboardHostSkip,
    LeaderboardPayload.LeaderboardHostSkipPayload,
    leaderboardMessage
  )

  websocketStore.sendMessage(
    buildMessage(builder, message, MessageType.Leaderboard, Payload.LeaderboardPayloadType)
  )
}

</script>
<template>
  <div v-if="websocketStore.isHosting">
    <div class="flex flex-col gap-4 w-full h-full">
      <PeoplePartyHeader />
      <!-- TODO: Make this justify-between and uncomment the skip component -->
      <div class="w-full flex justify-center px-8">
        <div>
          <TimeComponent :timeLeft="timeLeft" />
        </div>
        <div>
          <PartyButton @click="skipLeaderboard" class="ml-16">
            skip
          </PartyButton>
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
            <div class="w-full inline-flex justify-between text-2xl px-4">
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
  </div>
  <div v-if="!websocketStore.isHosting" class="flex flex-col gap-4 h-full">
    <div class="w-full flex justify-center mb-4 mt-2">
      <p class="text-5xl">Leaderboard</p>
    </div>
    <n-scrollbar id="playerLeaderboard" class="h-full">
      <div class="mx-auto mr-2 mb-4" v-for="(player, i) in sortedLeaderboard" :key="i">
        <n-card :style="player.name === websocketStore.clientName ? 'background-color: gray' : ''">
          <div class="w-full inline-flex justify-between text-2xl px-1">
            <p class="inline-flex">
              <span class="w-16">{{ formatOrdinals(i + 1) }}.</span
              ><span class="font-bold col-span-5">{{ player.name }}</span>
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
