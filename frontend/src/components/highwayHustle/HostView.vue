<script lang="ts" setup>
import { ref, defineProps } from 'vue'
import { type IntroductionData } from '@/components/introduction/Introduction.vue'
import Introduction from '@/components/introduction/Introduction.vue'
import {
  GameStateType,
  HighwayHustleResultPayload,
  MiniGameIntroductionPayload,
  type MiniGamePayloadType
} from '@/flatbuffers/messageClass'
import { type HighwayHustleData, type HighwayHustleResult } from './HighwayHustleModels'
import { parseHighwayHustleHostPayload } from './HighwayHustleProcessor'
import { Application } from 'vue3-pixi'
import { Graphics, Sprite } from 'pixi.js'
import { watch } from 'vue'
import { getPlayerSprite, getPlayerSpriteDimensions, getObstacleSprite, getObstacleDimensions} from './HighwayHustleSpriteMap'

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
const results = ref<HighwayHustleResult>({
  results: []
})

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
    case GameStateType.HighwayHustleResult: {
      viewState.value = ViewState.Results
      results.value = data.gamestatepayload(new HighwayHustleResultPayload())
      break;
    }
  }
  return []
}

// render the road dotted lines
const render = (graphics: Graphics) => {
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

const applicationId = ref(0)

watch(
  () => payloadData.value.obstacles,
  (oldVal, newVal) => {
    // if an object is removed, destroy it
    if (!oldVal || !newVal) return
    newVal.forEach((newObj) => {
      const oldObj = oldVal.find((obj) => obj.id === newObj.id)
      if (!oldObj) {
        // force rerender of the application
        applicationId.value += 1;
      }
    })
  },
  { immediate: true }
)

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
      <div class="flex flex-col h-full w-full justify-center items-center">
        <div>
          score {{ Math.round(payloadData.distance / 10) }}
        </div>
        <div class="relative">
          <!-- black background, for when the application is rerendering -->
          <div class="absolute top-0 left-0 w-full h-full bg-black"></div>
          <div class="relative">
            <Application :key="applicationId" :width="800" :height="530" background-color="black" >
              <Graphics :x="0" :y="0" @render="render" />
              <Sprite
                v-for="(entity) in payloadData.obstacles"
                :position="{ x: entity.x || 0, y: entity.y || 0 }"
                :width="getObstacleDimensions(entity.carType).width"
                :height="getObstacleDimensions(entity.carType).height"
                :key="entity.id"
                :texture=getObstacleSprite(entity.carType)
              />
              <Sprite
                v-for="(entity) in payloadData.players"
                :position="{ x: entity.x, y: entity.y }"
                :width="getPlayerSpriteDimensions(entity.carType).width"
                :height="getPlayerSpriteDimensions(entity.carType).height"
                :key="entity.id"
                :texture=getPlayerSprite(entity.carType)
              />
            </Application>
          </div>
        </div>
      </div>
    </div>
    <div v-else-if="viewState == ViewState.Results">
      <!-- todo -->
      <!-- show a F1 style like placement screen (same position as application on other screen) -->
    </div>
  </div>
</template>

<style scoped>
</style>