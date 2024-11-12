<script setup lang="ts">
import { useWebSocketStore } from '@/stores/confettiStore';
import { NInputNumber, NSwitch } from 'naive-ui'
import * as flatbuffers from 'flatbuffers'
import { MessageType, PartyPrepPayload, PartyPrepPayloadType, PartyPrepSettingsMiniGamePayload, PartyPrepSettingsRoundsPayload, PartyPrepType, Payload } from '@/flatbuffers/messageClass';
import { buildMessage } from '@/util/flatbufferMessageBuilder';

const websocketStore = useWebSocketStore()

const handleChange = (value: boolean) => {
    if (!value) {
        //websocketStore.partyPrepSettings.number_of_rounds = websocketStore.partyPrepSettings.minigames.length
        sendNewNumberOfRounds(websocketStore.partyPrepSettings.minigames.length)
    } else {
        //websocketStore.partyPrepSettings.number_of_rounds = 0
        sendNewNumberOfRounds(0)
    }
}

const handleNumberChange = (value: number) => {
    sendNewNumberOfRounds(value)
}

const nameToFirstLetterCapital = (name: string) => {
    return name.replace('_', ' ').split(' ').map(word => word.charAt(0).toUpperCase() + word.substring(1)).join(' ')
}

// flatbuffer functions
const sendToggleMiniGame = (name: string, enable: boolean) => {
    let builder = new flatbuffers.Builder()

    let miniGameMessage = PartyPrepSettingsMiniGamePayload.createPartyPrepSettingsMiniGamePayload(
        builder,
        builder.createString(name),
        enable
    )

    let message = PartyPrepPayloadType.createPartyPrepPayloadType(
        builder,
        PartyPrepType.PartyPrepSettingsMiniGame,
        PartyPrepPayload.PartyPrepSettingsMiniGamePayload,
        miniGameMessage
    )

    websocketStore.sendMessage(
        buildMessage(builder, message, MessageType.PartyPrep, Payload.PartyPrepPayloadType)
    )
}

const sendNewNumberOfRounds = (newNumber: number) => {
    let builder = new flatbuffers.Builder()

    let miniGameMessage = PartyPrepSettingsRoundsPayload.createPartyPrepSettingsRoundsPayload(
        builder,
        BigInt(newNumber)
    )

    let message = PartyPrepPayloadType.createPartyPrepPayloadType(
        builder,
        PartyPrepType.PartyPrepSettingsRounds,
        PartyPrepPayload.PartyPrepSettingsRoundsPayload,
        miniGameMessage
    )

    websocketStore.sendMessage(
        buildMessage(builder, message, MessageType.PartyPrep, Payload.PartyPrepPayloadType)
    )
}

</script>
<template>
  <div class="w-full h-full max-h-full">
    <div class="grid max-h-full h-full w-full">

        <div class="row-span-1 grid grid-cols-3">
            <div class="col-span-1">
                Ammount of minigames:
            </div>
            <div class="col-span-2 h-full w-full">
                <div class="grid grid-cols-2 mb-4">
                    <span>Loop:</span> 
                    <div><n-switch class="" v-model:value="websocketStore.partyPrepSettings.loop" @update:value="handleChange"/></div>
                    
                </div>
                <div v-if="!websocketStore.partyPrepSettings.loop" class="grid grid-cols-2 mb-4">
                    <span>Number of rounds (minigames): </span>
                    <n-input-number 
                        class="w-24"
                        placeholder="0"
                        :min="0"
                        :max="999"
                        v-model:value="websocketStore.partyPrepSettings.number_of_rounds"
                        @update:value="handleNumberChange"
                    >
                    </n-input-number>
                </div>
            </div>
        </div>

        <div class="row-span-1 grid grid-cols-3 mt-4">
            <div class="col-span-1">
                Sellect minigames:
            </div>
            <div class="col-span-2">
                <div class="mb-2">
                    <span>Click to enable or disable</span>
                </div>
                <div class="grid grid-cols-3 rounded-xl" style="background-color: rgb(0 0 0 / .4)">
                    <div v-for="(miniGame, i) in websocketStore.partyPrepSettings.minigames" :key="i" class="m-2 mb-1 mt-1">
                        <div :class="{ 'mt-2' : i < 3 }">
                            <button v-on:click="sendToggleMiniGame(miniGame.name, !miniGame.enabled)" class="h-full w-full">
                                <div v-if="miniGame.enabled" class="fixed h-full w-full z-40 rounded-xl" style="border: 5px solid green;"></div>
                                <div v-if="!miniGame.enabled" class="fixed h-full w-full z-40 rounded-xl" style="border: 5px solid red;"></div>

                                <div v-if="!miniGame.enabled" class="fixed h-full w-full z-30 rounded-xl" style="background-color: rgb(0 0 0 / .6)"></div>
                                <img
                                    v-if="miniGame.image !== ''"
                                    :src="miniGame.image" 
                                    alt="logo"
                                    class="h-full w-full rounded-xl"
                                />
                                
                                <div v-else class="h-full w-full flex justify-center items-center">
                                    <span class="text-2xl">{{ nameToFirstLetterCapital(miniGame.name) }}</span>
                                </div>
                            </button>
                        </div>
                        <div class="ml-2" style="margin-top: -0.4rem;">
                            <span class="text text-black">{{ nameToFirstLetterCapital(miniGame.name) }}</span>
                        </div>
                    </div>
                </div>
            </div>
        </div>

        <!-- <div class="row-span-1 grid grid-cols-3">
            <div class="col-span-1">
                The name of setting
            </div>
            <div class="col-span-2">
                the sellector / setting itself
            </div>
        </div> -->

    </div>
  </div>
</template>
<style scoped>
.n-input-number :deep(button), button {
  box-shadow: 0.0rem 0.0rem 0 #000 !important;
  transition:
    transform 0.1s ease-in-out,
    box-shadow 0.1s ease-in-out !important;
}

.n-input-number :deep(button):active, button {
  transform: translate(0.1rem, 0.1rem) !important;
  box-shadow: 0.0rem 0.0rem 0 #000 !important;
}
</style>