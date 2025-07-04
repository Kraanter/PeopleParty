<!-- eslint-disable vue/multi-word-component-names -->
<script setup lang="ts">
import { NScrollbar, NCard } from 'naive-ui'
import { defineProps, toRefs, computed, onMounted, ref, type VNodeRef } from 'vue'
import { useWebSocketStore, type Leaderboard } from '@/stores/confettiStore'
import PeoplePartyHeader from '@/components/PeoplePartyHeader.vue'
import TimeComponent from '../TimeComponent.vue'
import PartyButton from '../PartyButton.vue'

import * as flatbuffers from 'flatbuffers'
import {
  LeaderboardHostSkipPayload,
  LeaderboardPayload,
  LeaderboardPayloadType,
  LeaderboardType,
  MessageType,
  Payload
} from '@/flatbuffers/messageClass'
import { buildMessage } from '@/util/flatbufferMessageBuilder'

const props = defineProps<{
  data: Leaderboard
}>()

const websocketStore = useWebSocketStore()

const { data: leaderboard } = toRefs(props)

const sortedLeaderboard = computed(() => {
  const players = leaderboard?.value?.players ?? []
  return players.sort((a, b) => a.position - b.position)
})

const timeLeft = computed(() => {
  const time = leaderboard?.value?.time_left ?? 0
  return time
})

const scrollTo = () => {
  const playerCard = document.getElementById('playerCard')
  playerCard?.scrollIntoView({ behavior: 'smooth', block: 'center' })
}

onMounted(() => {
  scrollTo()
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

  let leaderboardMessage = LeaderboardHostSkipPayload.createLeaderboardHostSkipPayload(
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
      <div class="h-full w-full overflow-hidden">
        <div v-if="websocketStore.isPaused" class="fixed left-0 h-full w-full justify-center items-center z-30">
          <div style="background-color: rgb(0 0 0 / .4)" class="fixed flex h-full w-full justify-center items-center z-30">
            <div style="background-color: rgb(0 0 0 / .65);" class="rounded-xl mb-32">
              <span class="text-6xl text-white m-3 flex">Game is paused</span>
            </div>
          </div>
          <div class="fixed h-full w-full backdrop-blur-xl opacity-60"></div>
        </div>
      </div>
      <div class="w-full grid grid-cols-3 grid-rows-1 px-24">
        <div v-if="leaderboard.podium" class="col-start-1" >
          <span class="text-6xl text-black ml-12">Podium</span>
        </div>
        <div class="col-start-2 mx-auto">
          <TimeComponent :timeLeft="timeLeft" />
        </div>
        <div class="mx-auto">
          <PartyButton class="w-24" @click="skipLeaderboard">
            <svg
              xmlns="http://www.w3.org/2000/svg"
              viewBox="0 0 24 24"
              fill="currentColor"
              class="size-12 text-white mx-8"
            >
              <path
                d="M5.055 7.06C3.805 6.347 2.25 7.25 2.25 8.69v8.122c0 1.44 1.555 2.343 2.805 1.628L12 14.471v2.34c0 1.44 1.555 2.343 2.805 1.628l7.108-4.061c1.26-.72 1.26-2.536 0-3.256l-7.108-4.061C13.555 6.346 12 7.249 12 8.689v2.34L5.055 7.061Z"
              />
            </svg>
          </PartyButton>
        </div>
      </div>
      <div v-if="leaderboard.podium && sortedLeaderboard.length > 2" style="height: 75vh;">
        <!-- podium -->
        <n-scrollbar class="-mb-4">
          <div class="mx-auto mb-4 w-4/5 mt-4" v-for="(player, i) in sortedLeaderboard" :key="i">
            <!-- First 3 -->
            <div v-if="i == 0" class="grid grid-cols-3">
              <div v-for="(player2, i) in sortedLeaderboard.slice(0, 3)" :key="i" class="ml-8 mr-8" :class="{'pos-1': i == 0, 'pos-2': i == 1, 'pos-3': i == 2}">
                <n-card class="flex extended">
                  <div class="w-full text-2xl flex justify-center items-center">
                    <div class="flex justify-center items-center">
                      <span class="font-bold ml-4 text-4xl col-span-5 mt-8">{{ player2.name }}</span>
                    </div>
                    <div class="absolute bottom-0 mb-4">
                      <span class="font-bold">{{ formatOrdinals(i + 1) }}.</span> with
                      <span class="font-bold">{{ player2.score }}</span> Points
                    </div>
                  </div>
                </n-card>
              </div>
            </div>
            <!-- Rest of the board -->
            <div v-if="i > 2">
              <n-card>
                <div class="w-full inline-flex justify-between text-2xl px-4">
                  <p class="inline-flex">
                    <!-- don't show ordinals if placement is the same -->
                    <span class="w-16"><span v-if="i + 1 === player.position">{{ formatOrdinals(i + 1) }}.</span></span>
                    <span class="font-bold ml-4 col-span-5">{{ player.name }}</span>
                  </p>
                  <p>
                    <span class="font-bold">{{ player.score }}</span> Points
                  </p>
                </div>
              </n-card>
            </div>
          </div>
        </n-scrollbar>
      </div>
      <div v-else style="height: 75vh;">
        <!-- normal leaderboard-->
        <n-scrollbar class="h-full">
          <div class="mx-auto mb-4 w-4/5" v-for="(player, i) in sortedLeaderboard" :key="i">
            <n-card>
              <div class="w-full inline-flex justify-between text-2xl px-4">
                <p class="inline-flex">
                  <!-- don't show ordinals if placement is the same -->
                  <span class="w-16"><span v-if="i + 1 === player.position">{{ formatOrdinals(i + 1) }}.</span></span>
                  <span v-if="!leaderboard.podium"
                    class="w-12"
                    :class="player.deltaPosition > 0 ? 'text-green-600' : 'text-red-600'"
                  >
                    <span class="font-bold flex flex-row" v-if="player.deltaPosition > 0">
                      <svg
                        xmlns="http://www.w3.org/2000/svg"
                        fill="none"
                        viewBox="0 0 24 24"
                        stroke-width="1.5"
                        stroke="currentColor"
                        class="size-5 w-7 mt-1"
                      >
                        <path
                          stroke-linecap="round"
                          stroke-linejoin="round"
                          d="m4.5 15.75 7.5-7.5 7.5 7.5"
                        />
                      </svg>
                      <span> {{ Math.abs(player.deltaPosition) }}</span>
                    </span>
                    <span class="font-bold flex flex-row" v-else-if="player.deltaPosition < 0">
                      <svg
                        xmlns="http://www.w3.org/2000/svg"
                        fill="none"
                        viewBox="0 0 24 24"
                        stroke-width="1.5"
                        stroke="currentColor"
                        class="size-5 w-7 mt-1"
                      >
                        <path
                          stroke-linecap="round"
                          stroke-linejoin="round"
                          d="m19.5 8.25-7.5 7.5-7.5-7.5"
                        />
                      </svg>
                      <span> {{ Math.abs(player.deltaPosition) }}</span>
                    </span>
                  </span>
                  <span class="font-bold ml-4 col-span-5">{{ player.name }}</span>
                </p>
                <p>
                  <span class="font-bold">{{ player.score }}</span> Points
                  <span
                    v-if="!leaderboard.podium" 
                    class="text-base"
                    :class="player.deltaScore > 0 ? 'text-green-600' : 'text-gray-600'"
                  >
                    {{ player.deltaScore > 0 ? '+' : '-' }}
                    {{ player.deltaScore }}
                  </span>
                </p>
              </div>
            </n-card>
          </div>
        </n-scrollbar>
      </div>
    </div>
  </div>
  <div v-if="!websocketStore.isHosting" class="flex flex-col gap-4 h-full overflow">
    <!-- pause screen -->
    <div v-if="websocketStore.isPaused" class="fixed h-full w-full top-0 left-0 justify-center items-center z-30">
      <div style="background-color: rgb(0 0 0 / .4)" class="fixed flex h-full w-full justify-center items-center z-30">
        <div style="background-color: rgb(0 0 0 / .65);" class="rounded-xl m-4">
          <span class="text-6xl text-white m-3 flex">Game is paused</span>
        </div>
      </div>
      <div class="fixed h-full w-full backdrop-blur-xl opacity-60"></div>
    </div>
    <div class="w-full flex justify-center mb-4 mt-2">
      <p v-if="!leaderboard.podium" class="text-5xl">Leaderboard</p>
      <p v-else class="text-5xl">Podium</p>
    </div>
    <!-- mobile leaderboard -->
    <n-scrollbar id="playerLeaderboard" class="h-full">
      <div class="mx-auto mr-2 mb-4" v-for="(player, i) in sortedLeaderboard" :key="i">
        <n-card
          :id="player.name === websocketStore.clientName ? 'playerCard' : undefined"
          :style="
            player.name === websocketStore.clientName
              ? 'background-color: var(--color-primary-dark); color: white'
              : ''
          "
        >
          <div class="w-full inline-flex justify-between text-2xl px-1">
            <p class="inline-flex">
              <!-- don't show ordinals if placement is the same -->
              <span class="w-16"><span v-if="i + 1 === player.position">{{ formatOrdinals(i + 1) }}.</span></span>
              <span class="font-bold col-span-5">{{ player.name }}</span>
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
<style scoped>
.pos-1 {
  grid-column-start: 2;
  grid-row-start: 1;
  
  .extended {
    background-color: #ffd700;
    height: 40vh;
  }
}

.pos-2 {
  grid-column-start: 1;
  grid-row-start: 1;
  align-self: flex-end;
  
  .extended {
    background-color: #c0c0c0;
    height: 30vh;
  }
}

.pos-3 {
  grid-column-start: 3;
  grid-row-start: 1;
  align-self: flex-end;

  .extended {
    background-color: #cd7f32;
    height: 20vh;
  }
}
</style>