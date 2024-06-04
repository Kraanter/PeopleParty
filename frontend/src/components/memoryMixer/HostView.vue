<script lang="ts" setup>
import { ref, toRefs, defineProps } from 'vue'
import { type IntroductionData } from '@/components/introduction/Introduction.vue'
import { GameStateType, MemoryMixerGridPayload, MiniGameIntroductionPayload, type MiniGamePayloadType } from '@/flatbuffers/messageClass';
import Introduction from '@/components/introduction/Introduction.vue'
import GridView from './GridView.vue'
import { processGrid, type MemoryMixerGrid } from './GridProcessor';


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

const { width, height } = toRefs(props)

const grid = ref<MemoryMixerGrid>({
  timeLeft: 0,
  maxOnCard: 0,
  grid: [],
})


// introduction
const intro = ref<IntroductionData>({
  title: '',
  description: '',
  time_left: 0
})
// game data

const update = (data: MiniGamePayloadType) => {
  switch (data.gamestatetype()) {
    case GameStateType.MemoryMixerGrid: {
      viewState.value = ViewState.MiniGame
      const hostEntitiesPayload: MemoryMixerGridPayload = data.gamestatepayload(
        new MemoryMixerGridPayload()
      )

      grid.value = processGrid(hostEntitiesPayload)
      return null;
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
      return intro.value
    }
  }
  return []
}

defineExpose({
  update
})
</script>
<template>
  <div v-if="viewState == ViewState.Introduction">
    <Introduction :data="intro" logoSVG="/assets/games/crazyCounting/crazyCountingLogo.svg" />
  </div>
  <div v-else-if="viewState == ViewState.MiniGame" class="flex justify-stretch">
    <GridView 
      :grid="grid" 
      :player-submitted="{playerSubmitted: false, x: 0, y: 0}"
      :isHost="true"></GridView>
  </div>
  <div v-else-if="viewState == ViewState.Results">

  </div>
</template>
