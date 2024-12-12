<script setup lang="ts">
import PeoplePartyLogo from '@/components/PeoplePartyLogo.vue'
import { useWebSocketStore } from '@/stores/confettiStore'
import { storeToRefs } from 'pinia'
import * as flatbuffers from 'flatbuffers'
import { PausePayloadType } from '@/flatbuffers/pause-payload-type';
import { buildMessage } from '../util/flatbufferMessageBuilder'
import { MessageType } from '../flatbuffers/message-type';
import { Payload } from '../flatbuffers/payload';
import PartyButton from './PartyButton.vue'
import { ref } from 'vue';

const { partyCode } = storeToRefs(useWebSocketStore())
const websocketStore = useWebSocketStore()

const isOnCooldown = ref(false)

const togglePauseGame = () => {
  if (isOnCooldown.value) {
    return
  }

  let builder = new flatbuffers.Builder()

  let pausePayload = PausePayloadType.createPausePayloadType(
    builder,
    !websocketStore.isPaused
  )

  websocketStore.sendMessage(
    buildMessage(builder, pausePayload, MessageType.Pause, Payload.PausePayloadType)
  )

  isOnCooldown.value = true
  setTimeout(() => {
    isOnCooldown.value = false
  }, 1000)
}

</script>
<template>
  <div
    class="w-[95%] mx-auto bg-white grid grid-cols-3 grid-rows-1 rounded-lg mt-2 mb-8 px-4 max-h-20 h-full text-2xl justify-center items-center cartoonShadow"
  >
    <p>Join now at <span class="font-bold">peopleparty.nl</span></p>
    <div class="w-full row-span-3 -mt-4 h-28 my-auto">
      <PeoplePartyLogo />
    </div>
    <div class="grid grid-cols-2 grid-rows-1 justify-center items-center">
      <div class="h-full col-span-1 mr-8">
        <PartyButton class="text-xxl" @click="togglePauseGame">
            <span v-if="!websocketStore.isPaused">Pause</span>
            <span v-if="websocketStore.isPaused">Resume</span>
        </PartyButton>
      </div>
      <div class="h-full col-span-1">
        <p class="text-right">
          Party Code: <span class="font-extrabold">{{ partyCode }}</span>
        </p>
      </div>
    </div>
  </div>
</template>
