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

const websocketStore = useWebSocketStore()
const { partyCode } = storeToRefs(websocketStore)

const viewStore = useViewStore()
const { viewState, viewData } = storeToRefs(viewStore)

const host = () => {
  websocketStore.host()
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
    <div class="w-full max-w-screen-md h-full p-4 m-auto">
      <PeoplePartyLogo />
    </div>

    <div class="flex flex-col h-full justify-between p-8 items-center">
      <div id="partyButton">
        <PartyButton class="py-8 px-12" @click="host()">Host a party!</PartyButton>
      </div>

      <router-link to="/join" class="underline">Join a party!</router-link>
    </div>
  </div>
</template>
