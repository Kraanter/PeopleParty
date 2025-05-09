<script lang="ts" setup>
import { ref, defineProps } from 'vue'
import { type IntroductionData } from '@/components/introduction/Introduction.vue'
import Introduction from '@/components/introduction/Introduction.vue'
import {
  GameStateType,
  MiniGameIntroductionPayload,
  type MiniGamePayloadType
} from '@/flatbuffers/messageClass'
import { type HighwayHustleData, type HighwayHustleResult, type HighwayHustleResultPair } from './HighwayHustleModels'
import { parseHighwayHustleHostPayload, parseHighwayHustleResultPayload } from './HighwayHustleProcessor'
import { Application } from 'vue3-pixi'
import { Graphics, Sprite, Text, TextStyle, TextMetrics } from 'pixi.js'
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
      results.value = parseHighwayHustleResultPayload(data)
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
  const verticalSpacing = 90

  const offset = payloadData.value.distance % spacing

  for (let y = 0; y <= canvasHeight; y += verticalSpacing) {
    if (y >= 0 && y <= canvasHeight) {
      // draw a horizontal dotted line
      for (let x = -offset; x <= canvasWidth; x += spacing) {
        graphics.moveTo(x, y - 10)
        graphics.lineTo(x + spacing / 3, y - 10) // draw dash of half-spacing width
      }
    }
  }

  if (viewState.value == ViewState.Results) {
    // draw placement 'bracket'
    graphics.lineStyle(3, 0xFFFFFF)
    let localLeaderboardLocations: { x: number, y: number }[] = [];

    for (let i = 0; i < 12; i++) {
      let y = (i + 1) * verticalSpacing - verticalSpacing / 2 - 2 * i;
      if (y >= canvasHeight) y -= canvasHeight;

      const x = canvasWidth - (canvasWidth / 15) * i - 50;

      localLeaderboardLocations.push({ x, y })

      graphics.moveTo(x, y - 20)
      graphics.lineTo(x, y + 20)

      graphics.moveTo(x, y - 20)
      graphics.lineTo(x - 20, y - 20)

      graphics.moveTo(x, y + 20)
      graphics.lineTo(x - 20, y + 20)
    }

    playerLeaderboardLocations.value = localLeaderboardLocations;
  }
}

const playerLeaderboardLocations = ref<{ x: number, y: number }[]>([])

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

const getResultSpritePostions = (entity: HighwayHustleResultPair, index: number): { x: number, y: number } => {
  console.log('getResultSpritePostions', entity, index, playerLeaderboardLocations.value[index])
  if (!playerLeaderboardLocations.value[index]) {
    return { x: -100, y: 0 }
  }
  return {
    x: playerLeaderboardLocations.value[index].x 
      - getPlayerSpriteDimensions(payloadData.value.players.find(a => a.id === entity.name).carType).width * 1.5 
      - 10,
    y: playerLeaderboardLocations.value[index].y 
      - getPlayerSpriteDimensions(payloadData.value.players.find(a => a.id === entity.name).carType).height * 1.5 / 2}
}

const style = new TextStyle({
  fontFamily: ['Helvetica', 'Arial', 'sans-serif'],
  fontSize: 18,
  fill: 'white',
  stroke: '#000000',
  strokeThickness: 4,
});

const getCenteredTextPosition = (text: string, x: number): number => {
  const metrics = TextMetrics.measureText(text, style)
  return x - metrics.width / 2
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
    <div v-else-if="viewState == ViewState.MiniGame || viewState == ViewState.Results">
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
              <template v-if="viewState == ViewState.MiniGame && payloadData.players.length > 0">
                <Sprite
                  v-for="(entity) in payloadData.obstacles"
                  :position="{ x: entity.x || 0, y: entity.y || 0 }"
                  :width="getObstacleDimensions(entity.carType).width * 1.5"
                  :height="getObstacleDimensions(entity.carType).height * 1.5"
                  :key="entity.id"
                  :texture="getObstacleSprite(entity.carType)"
                />
                <template v-for="(entity) in payloadData.players" :key="entity.id">
                  <Sprite
                    :position="{ x: entity.x, y: entity.y }"
                    :width="getPlayerSpriteDimensions(entity.carType).width * 1.5"
                    :height="getPlayerSpriteDimensions(entity.carType).height * 1.5"
                    :texture="getPlayerSprite(entity.carType)"
                  />
                  <!-- // player name in minigame result -->
                  <Text
                    :position="{
                      x: getCenteredTextPosition(`${entity.id}`, entity.x) + getPlayerSpriteDimensions(entity.carType).width * 1.5 / 2,
                      y: entity.y - 18 * 1.5
                    }"
                    :text="`${entity.id}`"
                    :style="style"
                  />
                  </template>
              </template>
              <template v-if="viewState == ViewState.Results && results.results.length > 0 && playerLeaderboardLocations.length > 0">
                <template v-for="(entity, i) in results.results" :key="entity.name">
                  <Sprite
                    :position="getResultSpritePostions(entity, i)"
                    :width="getPlayerSpriteDimensions(payloadData.players.find(a => a.id === entity.name).carType).width * 1.5"
                    :height="getPlayerSpriteDimensions(payloadData.players.find(a => a.id === entity.name).carType).height * 1.5"
                    :texture="getPlayerSprite(payloadData.players.find(a => a.id === entity.name).carType)"
                  />
                  <!-- // player name in minigame result -->
                  <Text
                    :position="{
                      x: getCenteredTextPosition(`${entity.name}`, playerLeaderboardLocations[i].x) 
                        - getPlayerSpriteDimensions(payloadData.players.find(a => a.id === entity.name).carType).width * 1.5 / 2,
                      y: getResultSpritePostions(entity, i).y - 18 * 1.5
                    }"
                    :text="`${entity.name}`"
                    :style="style"
                  />
                  <Text
                    :position="{
                      x: playerLeaderboardLocations[i].x + 8,
                      y: playerLeaderboardLocations[i].y - 15
                    }"
                    :text="`${formatOrdinals(i + 1)}.`"
                    style="fill: white;"
                  />
                </template>
              </template>
            </Application>
          </div>
        </div>
      </div>
    </div>
  </div>
</template>

<style scoped>
</style>