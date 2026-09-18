<script lang="ts" setup>
import { ref, shallowRef } from 'vue'
import { type IntroductionData } from '@/components/introduction/Introduction.vue'
import Introduction from '@/components/introduction/Introduction.vue'
import {
  GameStateType,
  MiniGameIntroductionPayload,
  type MiniGamePayloadType
} from '@/flatbuffers/messageClass'
import type { MarbleManiaData, MarbleManiaResult } from './MarbleManiaModels'
import { parseMarbleManiaHostPayload, parseMarbleManiaResultPayload } from './MarbleManiaProcessor'
import MarbleManiaGameView from './MarbleManiaGameView.vue'
import MarbleManiaResultsView from './MarbleManiaResultsView.vue'

defineProps<{ width: number; height: number }>()

enum ViewState {
  None,
  Introduction,
  MiniGame,
  Results
}
const viewState = ref<ViewState>(ViewState.None)

const intro = ref<IntroductionData>({ title: '', description: '', time_left: 0 })
const results = ref<MarbleManiaResult>({ results: [] })

// shallowRef, not ref: reassigned ~20x/sec with a fresh nested entity graph that's only ever
// read wholesale by the child (imperatively, off the shared rAF loop) — a plain ref would
// pointlessly deep-proxy every entity object on every message.
const hostData = shallowRef<MarbleManiaData | null>(null)

const gameViewRef = ref<InstanceType<typeof MarbleManiaGameView>>()

// Shared across both children so a player's marble color stays identical from gameplay
// through to the results screen — each child mounting fresh with its own map would
// re-assign colors by whatever order it first encounters players in, which differs
// between gameplay (roughly id order) and results (placement order).
const playerColorMap = new Map<string, number>()
const colorPalette = [
  0x4ade80, // Bright green
  0x3b82f6, // Bright blue
  0xf97316, // Orange
  0xeab308, // Yellow
  0xa855f7, // Purple
  0x06b6d4, // Cyan
  0xef4444, // Red
  0xf59e0b, // Amber
  0x10b981, // Emerald
  0x8b5cf6, // Violet
  0xf43f5e, // Rose
  0x0ea5e9, // Light blue
  0x84cc16, // Lime
  0xf472b6, // Pink
  0x6366f1, // Indigo
  0x14b8a6 // Teal
]

const getPlayerColor = (playerId: string): number => {
  if (!playerColorMap.has(playerId)) {
    const colorIndex = playerColorMap.size % colorPalette.length
    playerColorMap.set(playerId, colorPalette[colorIndex])
  }
  return playerColorMap.get(playerId)!
}

const update = (data: MiniGamePayloadType) => {
  switch (data.gamestatetype()) {
    case GameStateType.MarbleManiaHost: {
      viewState.value = ViewState.MiniGame
      hostData.value = parseMarbleManiaHostPayload(data)
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
    case GameStateType.MarbleManiaResult: {
      viewState.value = ViewState.Results
      results.value = parseMarbleManiaResultPayload(data)
      break
    }
  }
  return []
}

defineExpose({
  update,
  ...(import.meta.env.DEV
    ? {
        // Mirrors update()'s real MarbleManiaHost dispatch (same synchronous viewState/hostData
        // assignment), but takes an already-parsed MarbleManiaData instead of a FlatBuffer —
        // for driving the real prop-passing path from a dev harness without building fake
        // FlatBuffers (the decode layer isn't part of what these fixes touch).
        __devUpdateHost: (d: MarbleManiaData) => {
          viewState.value = ViewState.MiniGame
          hostData.value = d
        },
        __devCameraY: () => gameViewRef.value?.__devCameraY?.(),
        __devMarblePos: (id: string) => gameViewRef.value?.__devMarblePos?.(id)
      }
    : {})
})
</script>

<template>
  <div class="h-full">
    <div v-if="viewState == ViewState.Introduction">
      <Introduction :data="intro" logoSVG="/assets/games/MarbleMania/MarbleManiaLogo.svg" />
    </div>

    <MarbleManiaGameView
      v-else-if="viewState == ViewState.MiniGame"
      ref="gameViewRef"
      :host-data="hostData"
      :get-player-color="getPlayerColor"
    />

    <MarbleManiaResultsView
      v-else-if="viewState == ViewState.Results"
      :results="results"
      :get-player-color="getPlayerColor"
    />
  </div>
</template>

<style scoped></style>
