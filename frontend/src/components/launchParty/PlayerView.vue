<script lang="ts" setup>
import { ref, defineProps } from 'vue'
import { type IntroductionData } from '@/components/introduction/Introduction.vue'
import TimeComponent from '../TimeComponent.vue'
import {
  GameStatePayload,
  GameStateType,
  LaunchPartyLightsPayload,
  LaunchPartyPlayerInputPayload,
  LaunchPartyPlayerTimePayload,
  LaunchPartyResultPayload,
  MessageType,
  MiniGameIntroductionPayload,
  MiniGamePayloadType,
  Payload
} from '@/flatbuffers/messageClass'
import {
  type LauncPartyResultPair,
  type LaunchPartyLights,
  type LaunchPartyPlayerFeedback,
  type LaunchPartyResults
} from './LaunchPartyProcessor'
import * as flatbuffers from 'flatbuffers'
import { buildMessage } from '@/util/flatbufferMessageBuilder'
import { useWebSocketStore } from '@/stores/confettiStore'
import LightsComponent from './LightsComponent.vue'
import PartyButton from '@/components/PartyButton.vue'

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
  practice_round: true,
  lights: -1
})

const playerFeedback = ref<LaunchPartyPlayerFeedback>({
  reaction_time: -10000
})

const resultsData = ref<LaunchPartyResults>({
  results: []
})

const personalResult = ref<LauncPartyResultPair>({
  player: '',
  reaction_time: -10000
})

let hasSendLag: boolean = false

const update = (data: MiniGamePayloadType) => {
  switch (data.gamestatetype()) {
    case GameStateType.LaunchPartyLights: {
      viewState.value = ViewState.MiniGame
      const lights: LaunchPartyLightsPayload = data.gamestatepayload(new LaunchPartyLightsPayload())

      if (lights.lights() == 4 && !hasSendLag) {
        hasSendLag = true
        sendPlayerAction(false)
      }

      lightsData.value = {
        practice_round: lights.pratice(),
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
      const miniGameResultPayload: LaunchPartyResultPayload = data.gamestatepayload(
        new LaunchPartyResultPayload()
      )

      const results: LauncPartyResultPair[] = []
      for (let i = 0; i < miniGameResultPayload.minigameResultsLength(); i++) {
        const result = miniGameResultPayload.minigameResults(i)
        results.push({
          player: decodeURI(result.name()) || '',
          reaction_time: Number(result.reactionTime())
        })

        if (result.name() == websocketStore.clientName) {
          personalResult.value = {
            player: decodeURI(result.name()) || '',
            reaction_time: Number(result.reactionTime())
          }
        }
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

  let playerInput = LaunchPartyPlayerInputPayload.createLaunchPartyPlayerInputPayload(
    builder,
    pressed
  )

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
  <template v-if="viewState == ViewState.Introduction">
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
  </template>
  <template v-else-if="viewState == ViewState.MiniGame">
    <div class="mt-32">
      <div class="flex flex-col m-2 text-center gap-4 h-full justify-center items-center">
        <div class="m-auto w-full">
          <p class="text-3xl text-center text-white mt-4">
            Press the button when all the lights hit green!
          </p>
        </div>
        <div v-if="lightsData.lights != -1" class="mt-4">
          <LightsComponent :value="lightsData.lights" />
        </div>
        <div class="m-auto mt-3">
          <PartyButton
            @mousedown="sendPlayerAction(true)"
            :disabled="playerFeedback.reaction_time != -10000"
          >
            <p class="text-4xl m-6">Launch!</p>
          </PartyButton>
        </div>
        <div v-if="lightsData.practice_round">
          <p class="text-2xl w-full text-center text-white mt-4">
            Warmup round, no points are given!
          </p>
        </div>
        <div v-if="playerFeedback.reaction_time != -10000" class="mt-4">
          <p class="text-3xl text-center text-white mt-2">
            <span v-if="playerFeedback.reaction_time < 0">Too early!</span>
            <span v-else
              >reaction time: <span class="font-bold">{{ playerFeedback.reaction_time }}</span
              >ms</span
            >
          </p>
        </div>
      </div>
    </div>
  </template>
  <template v-else-if="viewState == ViewState.Results">
    <div class="h-full w-full flex flex-cols mr-auto justify-center text-center mt-40">
      <p v-if="personalResult" class="text-4xl text-center text-white mt-4">
        <span v-if="personalResult.reaction_time >= 5000 && personalResult.reaction_time < 10000"
          >Too early</span
        >
        <span
          v-else-if="
            personalResult.reaction_time >= 10000 || personalResult.reaction_time == -10000
          "
          >Too late</span
        >
        <span v-else
          >reaction time: <span class="font-bold">{{ personalResult.reaction_time }}</span
          >ms</span
        >
      </p>
    </div>
  </template>
</template>
