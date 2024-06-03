<script lang="ts" setup>
import { ref, toRefs, defineProps } from 'vue'
import { type IntroductionData } from '@/components/introduction/Introduction.vue'
import { GameStateType, MemoryMixerGridPayload, MiniGameIntroductionPayload, type MiniGamePayloadType } from '@/flatbuffers/messageClass';
import TimeComponent from '../TimeComponent.vue'


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
    <div v-if="viewState == ViewState.Introduction" class="flex flex-col m-2 text-center gap-4 h-full justify-center items-center">
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
  <div v-else-if="viewState == ViewState.MiniGame" class="flex justify-stretch">

  </div>
  <div v-else-if="viewState == ViewState.Results">

  </div>
</template>
