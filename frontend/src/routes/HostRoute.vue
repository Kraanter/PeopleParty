<script lang="ts" setup>
import { useWebSocketStore } from '@/stores/confettiStore'
import GameManager from '@/components/GameManager.vue'
import PartyButton from '@/components/PartyButton.vue'
import PartyPauseCard from '@/components/partyManagment/PartyPauseCard.vue'
import PeoplePartyLogo from '@/components/PeoplePartyLogo.vue'
import { storeToRefs } from 'pinia'
import { ref } from 'vue'

const isPlaying = ref(false)

const websocketStore = useWebSocketStore()
const { isHost } = storeToRefs(websocketStore)

const host = () => {
  websocketStore.host()
}

const play = () => (isPlaying.value = true)
</script>
<template>
  <div v-if="isHost" class="grid grid-rows-4 grid-cols-1 jusitfy-center">
    <!-- <GameManager v-if="isPlaying" is-host /> -->

    <div class="w-full h-full p-4 my-auto">
      <PeoplePartyLogo />
    </div>

    <div class="lg:max-w-screen-lg row-span-2 max-w-[95%] m-auto">
      <PartyPauseCard @click="play" message="Start Game" />
    </div>
  </div>
  <div v-else class="grid grid-rows-2 pt-12">
    <div class="w-full max-w-screen-md h-full p-4 m-auto">
      <PeoplePartyLogo />
    </div>

    <div class="flex flex-col h-full justify-between p-8 items-center">
      <PartyButton type="primary" @click="host()">Host a party!</PartyButton>

      <router-link to="/join" class="underline">Join a party!</router-link>
    </div>
  </div>
</template>
