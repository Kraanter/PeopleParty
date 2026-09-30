<script setup lang="ts">
import { ref, shallowRef } from 'vue'
import { GameStateType } from '@/flatbuffers/game-state-type'
import { MiniGameIntroductionPayload } from '@/flatbuffers/mini-game-introduction-payload'
import type { MiniGamePayloadType } from '@/flatbuffers/mini-game-payload-type'
import type { IntroductionData } from '../introduction/Introduction.vue'
import Introduction from '@/components/introduction/Introduction.vue'
import { parseRpsBracketHostPayload } from './RpsBracketProcessor'
import type { RpsBracketMatch } from './RpsBracketModels'
import RpsBracketGameView from './RpsBracketGameView.vue'

defineProps<{
  width: number
  height: number
}>()

enum ViewState {
  None,
  Introduction,
  MiniGame,
  Results
}

// introduction
const intro = ref<IntroductionData>({
  title: '',
  description: '',
  time_left: 0
})

const viewState = ref<ViewState>(ViewState.None)
const matches = shallowRef<RpsBracketMatch[]>([])

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
    case GameStateType.RPSBracketHost: {
      if (viewState.value === ViewState.Results) break
      viewState.value = ViewState.MiniGame
      matches.value = parseRpsBracketHostPayload(payload).matches
      break
    }
    default:
      throw new Error(`Unknown gamestatetype: ${payload.gamestatetype()}`)
  }
}

defineExpose({ update })
</script>
<template>
  <template v-if="viewState == ViewState.Introduction">
    <div>
      <Introduction :data="intro" logoSVG="/assets/games/rpsBracket/rpsBracketLogo.svg" />
    </div>
  </template>
  <template v-else-if="viewState == ViewState.MiniGame">
    <RpsBracketGameView :matches="matches" />
  </template>
</template>
