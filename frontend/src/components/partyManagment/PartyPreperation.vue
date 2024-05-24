<script setup lang="ts">
import { NCard } from 'naive-ui'
import PartyQrCode from './PartyQrCode.vue'
import PeoplePartyLogo from '../PeoplePartyLogo.vue'
import PartyButton from '../PartyButton.vue'
import { computed, toRefs } from 'vue'
import type { Player } from '@/stores/confettiStore'

const props = defineProps<{
  partyCode: string
  data: Player[]
}>()
const { data, partyCode } = toRefs(props)

const playerCount = computed(() => data.value?.length ?? 0)

const emit = defineEmits(['click'])
</script>
<template>
  <div class="w-full h-full max-h-full">
    <div class="grid max-h-full grid-cols-3 gap-x-12 gap-y-4 grid-rows-5 mb-4 pb-8 h-full w-full">
      <div class="col-span-3 w-full h-full">
        <PeoplePartyLogo />
      </div>
      <div class="row-span-4 w-full h-full flex flex-col justify-between max-h-full">
        <n-card>
          <div class="w-full h-full flex flex-col justify-center items-center">
            <span class="text-2xl mb-2"
              >Join now at <span class="font-bold">peopleparty.nl</span></span
            >
            <span class="text-6xl text-center font-semibold">Party Code:</span>
            <span class="text-8xl text-center font-extrabold my-4">{{ partyCode }}</span>
            <PartyQrCode />
            <div class="mx-auto mt-4 text-2xl flex gap-4">
              <svg
                xmlns="http://www.w3.org/2000/svg"
                viewBox="0 0 24 24"
                fill="currentColor"
                class="w-8 h-8"
              >
                <path
                  fill-rule="evenodd"
                  d="M8.25 6.75a3.75 3.75 0 1 1 7.5 0 3.75 3.75 0 0 1-7.5 0ZM15.75 9.75a3 3 0 1 1 6 0 3 3 0 0 1-6 0ZM2.25 9.75a3 3 0 1 1 6 0 3 3 0 0 1-6 0ZM6.31 15.117A6.745 6.745 0 0 1 12 12a6.745 6.745 0 0 1 6.709 7.498.75.75 0 0 1-.372.568A12.696 12.696 0 0 1 12 21.75c-2.305 0-4.47-.612-6.337-1.684a.75.75 0 0 1-.372-.568 6.787 6.787 0 0 1 1.019-4.38Z"
                  clip-rule="evenodd"
                />
                <path
                  d="M5.082 14.254a8.287 8.287 0 0 0-1.308 5.135 9.687 9.687 0 0 1-1.764-.44l-.115-.04a.563.563 0 0 1-.373-.487l-.01-.121a3.75 3.75 0 0 1 3.57-4.047ZM20.226 19.389a8.287 8.287 0 0 0-1.308-5.135 3.75 3.75 0 0 1 3.57 4.047l-.01.121a.563.563 0 0 1-.373.486l-.115.04c-.567.2-1.156.349-1.764.441Z"
                />
              </svg>
              {{ playerCount }}
            </div>
          </div>
        </n-card>
        <PartyButton id="startGame" @click="emit('click')" class="mt-8">Start Game </PartyButton>
      </div>
      <div
        class="col-span-2 row-span-4 max-h-full h-full overflow-y-auto backdrop-blur-xl p-4 rounded-3xl"
      >
        <div v-if="data.length > 0" class="grid grid-cols-2 gap-4">
          <n-card v-for="(player, i) in data" :key="i" class="font-bold text-center">
            <span class="text-2xl">
              {{ player.name }}
            </span>
          </n-card>
        </div>
        <div v-else class="flex items-center justify-center h-full">
          <span class="text-4xl font-semibold">
            Join using the party code on the left!
          </span>
        </div>
      </div>
    </div>
  </div>
</template>
