<script setup lang="ts">
import { GameStateType } from '@/flatbuffers/game-state-type'
import type { MiniGamePayloadType } from '@/flatbuffers/mini-game-payload-type'
import { MiniGameIntroductionPayload } from '@/flatbuffers/mini-game-introduction-payload'
import { parseBusinessBailoutPlayerPayload } from './parser'
import { ref } from 'vue'

enum ViewState {
  None,
  Introduction,
  MiniGame,
  Result
}
const viewState = ref<ViewState>(ViewState.None)

type IntroductionData = {
  title: string
  description: string
  time_left: number
}
const intro = ref<IntroductionData>({
  title: '',
  description: '',
  time_left: 0
})

const value = ref(0)

function update(payload: MiniGamePayloadType) {
  switch (payload.gamestatetype()) {
    case GameStateType.MiniGameIntroduction: {
      viewState.value = ViewState.Introduction
      const introPayload: MiniGameIntroductionPayload = payload.gamestatepayload(
        new MiniGameIntroductionPayload()
      )
      intro.value = {
        title: introPayload.name() || '',
        description: introPayload.instruction() || '',
        time_left: Number(introPayload.timeLeft())
      }
      break
    }
    case GameStateType.BusinessBailoutPlayer: {
      viewState.value = ViewState.MiniGame
      const { value: newValue } = parseBusinessBailoutPlayerPayload(payload)

      value.value = newValue
    }
  }
}

defineExpose({ update })
</script>
<template>
  <div>
    <h1>{{ value }}</h1>
  </div>
</template>
