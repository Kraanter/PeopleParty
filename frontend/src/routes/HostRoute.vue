<script lang="ts" setup>
import { useWebSocketStore } from '@/stores/confettiStore'
import GameManager from '@/components/GameManager.vue'
import PartyButton from '@/components/PartyButton.vue'
import PartyPreperation from '@/components/partyManagment/PartyPreperation.vue'
import PeoplePartyLogo from '@/components/PeoplePartyLogo.vue'
import { storeToRefs } from 'pinia'
import { onMounted, ref } from 'vue'
import * as flatbuffers from 'flatbuffers'
import { buildMessage } from '@/util/flatbufferMessageBuilder'
import {
  MessageType,
  PartyPrepHostStartGamePayload,
  PartyPrepPayload,
  PartyPrepPayloadType,
  PartyPrepType,
  Payload
} from '@/flatbuffers/messageClass'

const isPlaying = ref(false)

const websocketStore = useWebSocketStore()
const { isHost, partyCode } = storeToRefs(websocketStore)

const host = () => {
  websocketStore.host()
}

onMounted(() => {
  const unsubscribe = websocketStore.subscribe((roomId: string) => {
    partyCode.value = roomId
    console.log('Party code:', roomId)
  })

  // Unsubscribe when component is unmounted
  return unsubscribe
})

const startGame = () => {
  let builder = new flatbuffers.Builder()

  let startMessage = PartyPrepHostStartGamePayload.createPartyPrepHostStartGamePayload(builder, true);
  let payload = PartyPrepPayloadType.createPartyPrepPayloadType(builder, PartyPrepType.PartyPrepHostStartGame, PartyPrepPayload.PartyPrepHostStartGamePayload, startMessage);

  websocketStore.sendMessage(buildMessage(builder, payload, MessageType.PartyPrep, Payload.PartyPrepPayloadType))
  isPlaying.value = true;
}

const generateURL = () => `${window.location.origin}/join?code=${partyCode.value}`

</script>
<template>
  <div v-if="isHost" class="w-full h-full">
    <!-- <div class="w-full h-full p-4 my-auto"> -->
    <!-- <PeoplePartyLogo /> -->
    <!-- </div> -->

    <GameManager v-if="isPlaying" is-host />
    <div v-else class="max-w-[95%] h-full m-auto">
      <PartyPreperation @click="startGame()" />
    </div>
  </div>
  <div v-else class="grid grid-rows-2 pt-12">
    <div class="w-full max-w-screen-md h-full p-4 m-auto">
      <PeoplePartyLogo />
    </div>

    <div class="flex flex-col h-full justify-between p-8 items-center">
      <div>
        <PartyButton class="py-8 px-12" @click="host()">Host a party!</PartyButton>
      </div>

      <router-link to="/join" class="underline">Join a party!</router-link>
    </div>
  </div>
</template>
