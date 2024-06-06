<script lang="ts" setup>
import { ref, toRefs, defineProps } from 'vue'
import TimeComponent from '../TimeComponent.vue'
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
  Results,
  GridView
}

const viewState = ref<ViewState>(ViewState.GridView)

const { width, height } = toRefs(props)

const grid = ref<MemoryMixerGrid>({
  timeLeft: 0,
  maxOnCard: -1,
  phase: -1,
  grid: [],
})


// introduction
const intro = ref<IntroductionData>({
  title: '',
  description: '',
  time_left: 0
})
const submittedPlayers = ref<string[]>([])

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
    <Introduction :data="intro" logoSVG="/assets/games/memoryMixer/memoryMixerLogo.svg" />
  </div>
  <div v-else-if="viewState == ViewState.MiniGame" class="flex justify-stretch">
    <div class="mt-4 w-full h-full flex flex-col justify-center items-center">
        <div class="mx-auto mb-4">
          <TimeComponent :time-left="grid.timeLeft" />
        </div>
        <div class="mx-auto mt-4">
          <GridView 
          :grid="grid" 
          :player-submitted="{playerSubmitted: false, x: -1, y: -1}"
          :isHost="true">
          </GridView>
        </div>
      </div>
  </div>
  <div v-else-if="viewState == ViewState.Results">

  </div>
</template>
