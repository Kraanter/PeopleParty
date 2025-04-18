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
import { type HighwayHustleData } from './HighwayHustleModels'
import { parseHighwayHustleHostPayload } from './HighwayHustleProcessor'
import { Application } from 'vue3-pixi'
import { Graphics } from 'pixi.js'

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
  obstacles: [],
  distance: 0,
})

// const interpolatePosition = (entity: HighwayHustlePlayer): HighwayHustlePlayer => {
//   return {
//     x: entity.x * (500 - 30),
//     y: entity.y * (500 - 30)
//   }
// }

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

const getVisibleDottedLines = (distance: number, spacing = 50, canvasWidth = 750): number[] => {
  const positions: number[] = [];

  // Find where the first line appears on screen
  const offset = distance % spacing;

  // Start at -offset and add lines until we're past canvasWidth
  for (let x = -offset; x <= canvasWidth; x += spacing) {
    if (x >= 0 && x <= canvasWidth) {
      positions.push(x);
    }
  }

  return positions;
}

// render the road dotted lines
function render(graphics: Graphics) {
  graphics.clear()
  graphics.lineStyle(3, 0xffffff)

  const canvasWidth = 780
  const canvasHeight = 530
  const spacing = 150
  const horizontalSpacing = 90

  const offset = payloadData.value.distance % spacing

  for (let y = 0; y <= canvasHeight; y += horizontalSpacing) {
    if (y >= 0 && y <= canvasHeight) {
      // draw a horizontal dotted line
      for (let x = -offset; x <= canvasWidth; x += spacing) {
        graphics.moveTo(x, y - 10)
        graphics.lineTo(x + spacing / 3, y - 10) // draw dash of half-spacing width
      }
    }
  }
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
      <div class="flex flex-col h-full w-full justify-center items-center">
        <div>
          {{ payloadData.distance }}
          {{ getVisibleDottedLines(payloadData.distance) }}
          {{ payloadData.players.length }}
          {{ payloadData.obstacles.length }}
        </div>
        <Application key="gameview" :width="780" :height="530" background-color="black" >
          <Graphics :x="0" :y="0" @render="render" />
          <sprite
            v-for="(entity, i) in payloadData.obstacles"
            :position="{ x: entity.x, y: entity.y }"
            :width="30"
            :height="30"
            :key="i"
            texture="/assets/games/crazyCounting/partyhat.svg"
          />
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
    </div>
    
    <div v-else-if="viewState == ViewState.Results">
      <!-- todo -->
    </div>
  </div>
</template>

<style scoped>
</style>