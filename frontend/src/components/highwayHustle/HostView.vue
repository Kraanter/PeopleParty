<script lang="ts" setup>
import { ref, defineProps } from 'vue'
import { type IntroductionData } from '@/components/introduction/Introduction.vue'
import Introduction from '@/components/introduction/Introduction.vue'
import {
FBHighwayHustleEntity,
  GameStateType,
  MiniGameIntroductionPayload,
  type MiniGamePayloadType
} from '@/flatbuffers/messageClass'
import { type HighwayHustleData, type HighwayHustlePlayer } from './HighwayHustleModels'
import { parseHighwayHustleHostPayload } from './HighwayHustleProcessor'
import { Application } from 'vue3-pixi'

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
const payloadData = ref<HighwayHustleData>({
  players: [],
})

const interpolatePosition = (entity: HighwayHustlePlayer): HighwayHustlePlayer => {
  return {
    x: entity.x * (500 - 30),
    y: entity.y * (500 - 30)
  }
}

// final results
// todo

const update = (data: MiniGamePayloadType) => {
  switch (data.gamestatetype()) {
    case GameStateType.HighwayHustleHost: {
      viewState.value = ViewState.MiniGame

      payloadData.value = parseHighwayHustleHostPayload(data)
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

defineExpose({
  update
})

</script>
<template>
  <div class="h-full">
    <div v-if="viewState == ViewState.Introduction">
      <Introduction :data="intro" logoSVG="/assets/games/highwayHustle/highwayHustleLogo.svg" />
    </div>
    <div v-else-if="viewState == ViewState.MiniGame">
      {{ payloadData.players }}
      <Application key="gameview" :width="530" :height="530" background-color="white">
        <sprite
          v-for="(entity, i) in payloadData.players"
          :position="{ x: entity.x, y: entity.y }"
          :width="30"
          :height="30"
          :key="i"
          texture="/assets/games/crazyCounting/partyhat.svg"
        />
      </Application>
    </div>
    
    <div v-else-if="viewState == ViewState.Results">
      <!-- todo -->
    </div>
  </div>
</template>

<style scoped>
</style>