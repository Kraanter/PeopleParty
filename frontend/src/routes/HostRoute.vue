<script lang="ts" setup>
import { useWebSocketStore } from '@/stores/confettiStore'
import { useViewStore, ViewState } from '@/stores/viewStore'
import GameManager from '@/components/GameManager.vue'
import PartyButton from '@/components/PartyButton.vue'
import PartyPreperation from '@/components/partyManagment/PartyPreperation.vue'
import Leaderboard from '@/components/leaderboard/Leaderboard.vue'
import PeoplePartyLogo from '@/components/PeoplePartyLogo.vue'
import { storeToRefs } from 'pinia'
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
import { watch } from 'vue'
import { useReleasesStore } from '@/stores/releasesStore'
import { format } from 'date-fns';

const audio = new Audio('/soundtrack.wav')
audio.loop = true
audio.volume = 0.5

const websocketStore = useWebSocketStore()
const { partyCode } = storeToRefs(websocketStore)

watch(() => websocketStore.partyPrepSettings?.music_volume, (newValue) => {
  if (audio.volume !== newValue) {
    audio.volume = newValue;
  }
})

const viewStore = useViewStore()
const { viewState, viewData } = storeToRefs(viewStore)

const releasesStore = useReleasesStore()

const host = () => {
  websocketStore.host()
  audio.play()
}

const startGame = () => {
  let builder = new flatbuffers.Builder()

  let startMessage = PartyPrepHostStartGamePayload.createPartyPrepHostStartGamePayload(
    builder,
    true
  )
  let payload = PartyPrepPayloadType.createPartyPrepPayloadType(
    builder,
    PartyPrepType.PartyPrepHostStartGame,
    PartyPrepPayload.PartyPrepHostStartGamePayload,
    startMessage
  )

  websocketStore.sendMessage(
    buildMessage(builder, payload, MessageType.PartyPrep, Payload.PartyPrepPayloadType)
  )
}

const skipLeaderboard = () => {
  // todo: send message to skip leaderboard
}
</script>
<template>
  <div v-if="viewState !== ViewState.None" class="w-full h-full">
    <GameManager
      :data="viewData"
      id="gameManager"
      v-if="viewState === ViewState.MiniGame"
      is-host
    />
    <Leaderboard
      v-if="viewState == ViewState.Leaderboard"
      :data="viewData"
      @click="skipLeaderboard()"
    />
    <PartyPreperation
      v-else-if="viewState == ViewState.PartyPrep"
      :data="viewData"
      :partyCode
      id="partyPrep"
      @click="startGame()"
    />
  </div>
  <div v-else class="grid grid-rows-2 pt-12">
    <div class="fixed size-96" style="right: 10%; top: 40%; transform: rotate(10deg);">
      <img src="/assets/update-sticker.svg" class="absolute mt-2" style="z-index: -10; transform: rotate(-8deg);" />
      <div class="flex flex-col justify-center items-center mt-28">
        <div>
          <span class="text text-4xl">
            <router-link v-if="releasesStore.releases[0].date" to="/releases" class="underline text text-white font-bold" target="_blank">
              New version!
            </router-link>
          </span>
        </div>
        <div class="mt-4 text text-white">
          <span>{{ format(releasesStore.releases[0].date, 'dd MMMM yyyy') }}</span>
        </div>
      </div>
    </div>
    <div class="w-full max-w-screen-md h-full p-4 m-auto">
      <PeoplePartyLogo />
    </div>
    <div class="flex flex-col h-full justify-between p-8 items-center">
      <div id="partyButton">
        <PartyButton class="py-8 px-12" @click="host()">Host a party!</PartyButton>
      </div>

      <div class="grid grid-cols-3 justify-between w-full">
        <router-link to="/releases" class="flex underline" target="_blank">{{ releasesStore.versionNumber }}</router-link>
        <div class="flex justify-center items-center">
          <router-link to="/join" class="underline">Join a party!</router-link>
        </div>
      </div>
    </div>
  </div>
</template>