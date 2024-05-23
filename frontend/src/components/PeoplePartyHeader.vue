<script setup lang="ts">
import PeoplePartyLogo from '@/components/PeoplePartyLogo.vue'
import { NButton } from 'naive-ui'
import { useWebSocketStore } from '@/stores/confettiStore'
import { storeToRefs } from 'pinia'

const { partyCode } = storeToRefs(useWebSocketStore())
</script>
<template>
  <div
    class="w-full bg-white grid grid-cols-3 grid-rows-1 rounded-lg mt-2 mb-8 px-4 max-h-20 h-full text-2xl justify-center items-center cartoonShadow"
  >
    <p>Join now at <span class="font-bold">peopleparty.nl</span></p>
    <div class="w-full row-span-3 -mt-4 h-28 my-auto">
      <PeoplePartyLogo />
    </div>
    <p class="text-right mr-4">
      Party Code: <span class="font-extrabold mr-4">{{ partyCode }}</span>
<!--      TODO: Put this button back when we have pausing figured out-->
<!--      <n-button type="primary" class="w-full h-full" circle>-->
<!--        <svg-->
<!--          xmlns="http://www.w3.org/2000/svg"-->
<!--          viewBox="0 0 24 24"-->
<!--          fill="currentColor"-->
<!--          class="w-6 h-6"-->
<!--        >-->
<!--          <path-->
<!--            fill-rule="evenodd"-->
<!--            d="M6.75 5.25a.75.75 0 0 1 .75-.75H9a.75.75 0 0 1 .75.75v13.5a.75.75 0 0 1-.75.75H7.5a.75.75 0 0 1-.75-.75V5.25Zm7.5 0A.75.75 0 0 1 15 4.5h1.5a.75.75 0 0 1 .75.75v13.5a.75.75 0 0 1-.75.75H15a.75.75 0 0 1-.75-.75V5.25Z"-->
<!--            clip-rule="evenodd"-->
<!--          />-->
<!--        </svg>-->
<!--      </n-button>-->
    </p>
  </div>
</template>
