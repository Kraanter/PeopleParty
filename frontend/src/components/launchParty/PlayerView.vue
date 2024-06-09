<script lang="ts" setup>
import { ref, defineProps } from 'vue'
import { type IntroductionData } from '@/components/introduction/Introduction.vue'
import TimeComponent from '../TimeComponent.vue'
import { GameStatePayload, GameStateType, LaunchPartyLightsPayload, LaunchPartyPlayerInputPayload, LaunchPartyPlayerTimePayload, LaunchPartyResultPayload, MessageType, MiniGameIntroductionPayload, MiniGamePayloadType, Payload } from '@/flatbuffers/messageClass';
import { type LauncPartyResultPair, type LaunchPartyLights, type LaunchPartyPlayerFeedback, type LaunchPartyResults } from './LaunchPartyProcessor';
import * as flatbuffers from 'flatbuffers'
import { buildMessage } from '@/util/flatbufferMessageBuilder';
import { useWebSocketStore } from '@/stores/confettiStore';
import LightsComponent from './LightsComponent.vue'

const websocketStore = useWebSocketStore()

const props = defineProps<{
  width: number
  height: number
}>()

enum ViewState {
  None,
  Introduction,
  MiniGame,
  Results
}

const viewState = ref<ViewState>(ViewState.None)

// introduction
const intro = ref<IntroductionData>({
  title: '',
  description: '',
  time_left: 0
})

const lightsData = ref<LaunchPartyLights>({
  lights: -1,
})

const playerFeedback = ref<LaunchPartyPlayerFeedback>({
  reaction_time: -1
})

const resultsData = ref<LaunchPartyResults>({
    results: []
})

let hasSendLag: boolean = false;

const update = (data: MiniGamePayloadType) => {
  switch (data.gamestatetype()) {
    case GameStateType.LaunchPartyLights: {
      viewState.value = ViewState.MiniGame
      const lights: LaunchPartyLightsPayload = data.gamestatepayload(
        new LaunchPartyLightsPayload()
      )

      if (lights.lights() == 4 && !hasSendLag) {
        hasSendLag = true
        sendPlayerAction(false)
      }

      lightsData.value = {
        lights: Number(lights.lights())
      }
      break
    }
    case GameStateType.LaunchPartyPlayerTime: {
      const playerFeedbackPayload: LaunchPartyPlayerTimePayload = data.gamestatepayload(
        new LaunchPartyPlayerTimePayload()
      )

      playerFeedback.value = {
        reaction_time: Number(playerFeedbackPayload.time())
      }
      break
    }
    case GameStateType.LaunchPartyResult: {
      viewState.value = ViewState.Results
      const miniGameResultPayload: LaunchPartyResultPayload = data.gamestatepayload(new LaunchPartyResultPayload())

      const results: LauncPartyResultPair[] = []
      for(let i = 0; i < miniGameResultPayload.minigameResultsLength(); i++) {
        const result = miniGameResultPayload.minigameResults(i)
        results.push({
          player: result.name() || '',
          reaction_time: Number(result.reactionTime())
        })
      }

      resultsData.value = {
        results: results
      }
      break
    }
    case GameStateType.MiniGameIntroduction: {
      viewState.value = ViewState.Introduction
      const introPayload: MiniGameIntroductionPayload = data.gamestatepayload(
        new MiniGameIntroductionPayload()
      )
      intro.value = {
        title: introPayload.name() || '',
        description: introPayload.instruction() || '',
        time_left: Number(introPayload.timeLeft())
      }
      break
    }
  }
  return []
}

const sendPlayerAction = (pressed: boolean) => {
  let builder = new flatbuffers.Builder()

  let playerInput = LaunchPartyPlayerInputPayload.createLaunchPartyPlayerInputPayload(builder, pressed)

  let miniGame = builder.createString('launchParty')

  let miniGamePayload = MiniGamePayloadType.createMiniGamePayloadType(
    builder,
    miniGame,
    GameStateType.LaunchPartyPlayerInput,
    GameStatePayload.LaunchPartyPlayerInputPayload,
    playerInput
  )

  websocketStore.sendMessage(
    buildMessage(builder, miniGamePayload, MessageType.MiniGame, Payload.MiniGamePayloadType)
  )
}

defineExpose({
  update
})
</script>
<template>
  <div class="h-full">
    <div v-if="viewState == ViewState.Introduction">
      <div class="flex flex-col m-2 text-center gap-4 h-full justify-center items-center">
        <div class="w-full flex justify-center px-8">
          <div>
            <TimeComponent :timeLeft="intro.time_left" />
          </div>
        </div>
        <div>
          <div class="w-full h-full mt-16">
            <p class="text-4xl text-white">{{ intro.description }}</p>
          </div>
        </div>
      </div>
    </div>
    <div v-else-if="viewState == ViewState.MiniGame">
      <div>
        <div v-if="lightsData.lights != -1">
          <LightsComponent :value="lightsData.lights" />
        </div>
        <button @click="sendPlayerAction(true)">Start!</button>
        <div v-if="playerFeedback.reaction_time != -1">
          {{  playerFeedback.reaction_time }}
        </div>
      </div>
    </div>
    <div v-else-if="viewState == ViewState.Results">
      
    </div>
  </div>
</template>
