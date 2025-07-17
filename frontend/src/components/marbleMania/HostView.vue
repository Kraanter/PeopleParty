<script lang="ts" setup>
import { ref, defineProps } from 'vue'
import { type IntroductionData } from '@/components/introduction/Introduction.vue'
import Introduction from '@/components/introduction/Introduction.vue'
import {
  GameStateType,
  MiniGameIntroductionPayload,
  type MiniGamePayloadType
} from '@/flatbuffers/messageClass'
import { type MarbleManiaData, type MarbleManiaResult, type MarbleManiaResultPair } from './MarbleManiaModels'
import { parseMarbleManiaHostPayload, parseMarbleManiaResultPayload } from './MarbleManiaProcessor'
import { Application } from 'vue3-pixi'
import { Graphics, Sprite, Text, TextStyle, CanvasTextMetrics } from 'pixi.js'
import { watch } from 'vue'

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

// host payload data
const payloadData = ref<MarbleManiaData>({
  entities: [],
  finish_line_y: 0,
  game_phase: 0,
  placement_time_left: 0,
})

// final results
const results = ref<MarbleManiaResult>({
  results: []
})

const update = (data: MiniGamePayloadType) => {
  switch (data.gamestatetype()) {
    case GameStateType.MarbleManiaHost: {
      viewState.value = ViewState.MiniGame

      payloadData.value = parseMarbleManiaHostPayload(data)
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
      break;
    }
  }
  return []
}

// render the road dotted lines
const render = (graphics: Graphics) => {
  graphics.clear()
}

const pr = new Intl.PluralRules('en-US', { type: 'ordinal' })
const suffixes = new Map([
  ['one', 'st'],
  ['two', 'nd'],
  ['few', 'rd'],
  ['other', 'th']
])

const formatOrdinals = (n: number) => {
  const rule = pr.select(n)
  const suffix = suffixes.get(rule)
  return `${n}${suffix}`
}

defineExpose({
  update
})

</script>
<template>
  <div class="h-full">
    <div v-if="viewState == ViewState.Introduction">
      <Introduction :data="intro" logoSVG="/assets/games/MarbleMania/MarbleManiaLogo.svg" />
    </div>
    <div v-else-if="viewState == ViewState.MiniGame">
      <div class="flex flex-col h-full w-full justify-center items-center">

      </div>
    </div>
    <div v-else-if="viewState == ViewState.Results" class="flex flex-col h-full w-full justify-center items-center">
      
    </div>
  </div>
</template>

<style scoped>
</style>