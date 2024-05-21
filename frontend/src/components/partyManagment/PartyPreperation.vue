<script setup lang="ts">
import { NCard, NButton, NList, NListItem } from 'naive-ui'
import PartyQrCode from './PartyQrCode.vue'
import { useWebSocketStore } from '@/stores/confettiStore'
import { storeToRefs } from 'pinia'
import PeoplePartyLogo from '../PeoplePartyLogo.vue'

const confettiStore = useWebSocketStore()
const { partyCode, playerCount, players } = storeToRefs(confettiStore)
</script>
<template>
  <div class="grid grid-cols-3 gap-12 grid-rows-1 h-full w-full">
    <n-card class="my-auto">
      <div class="w-full h-full flex flex-col justify-center items-center">
        <span class="text-lg mb-2">Join now at <span class="font-bold">peopleparty.nl</span></span>
        <span class="text-5xl text-center font-semibold">Party Code:</span>
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

    <div class="flex flex-col col-span-2 w-full justify-stretch items-stretch gap-4">
      <div class="grid grid-cols-6 w-full items-baseline px-4">
        <n-button type="primary" class="w-full h-full" circle
          ><svg
            xmlns="http://www.w3.org/2000/svg"
            viewBox="0 0 24 24"
            fill="currentColor"
            class="w-6 h-6"
          >
            <path
              fill-rule="evenodd"
              d="M11.078 2.25c-.917 0-1.699.663-1.85 1.567L9.05 4.889c-.02.12-.115.26-.297.348a7.493 7.493 0 0 0-.986.57c-.166.115-.334.126-.45.083L6.3 5.508a1.875 1.875 0 0 0-2.282.819l-.922 1.597a1.875 1.875 0 0 0 .432 2.385l.84.692c.095.078.17.229.154.43a7.598 7.598 0 0 0 0 1.139c.015.2-.059.352-.153.43l-.841.692a1.875 1.875 0 0 0-.432 2.385l.922 1.597a1.875 1.875 0 0 0 2.282.818l1.019-.382c.115-.043.283-.031.45.082.312.214.641.405.985.57.182.088.277.228.297.35l.178 1.071c.151.904.933 1.567 1.85 1.567h1.844c.916 0 1.699-.663 1.85-1.567l.178-1.072c.02-.12.114-.26.297-.349.344-.165.673-.356.985-.57.167-.114.335-.125.45-.082l1.02.382a1.875 1.875 0 0 0 2.28-.819l.923-1.597a1.875 1.875 0 0 0-.432-2.385l-.84-.692c-.095-.078-.17-.229-.154-.43a7.614 7.614 0 0 0 0-1.139c-.016-.2.059-.352.153-.43l.84-.692c.708-.582.891-1.59.433-2.385l-.922-1.597a1.875 1.875 0 0 0-2.282-.818l-1.02.382c-.114.043-.282.031-.449-.083a7.49 7.49 0 0 0-.985-.57c-.183-.087-.277-.227-.297-.348l-.179-1.072a1.875 1.875 0 0 0-1.85-1.567h-1.843ZM12 15.75a3.75 3.75 0 1 0 0-7.5 3.75 3.75 0 0 0 0 7.5Z"
              clip-rule="evenodd"
            />
          </svg>
        </n-button>
        <div class="col-span-4 w-full h-48">
          <PeoplePartyLogo />
        </div>
        <n-button type="primary" class="w-full h-full"> Start Game </n-button>
      </div>
      <div class="h-full bg-white p-4">
        <n-list>
          <n-list-item v-for="(player, i) in players" :key="i">
            <p>{{ player }}</p>
          </n-list-item>
        </n-list>
      </div>
    </div>
  </div>
</template>
