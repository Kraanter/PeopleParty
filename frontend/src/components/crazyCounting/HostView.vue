<script lang="ts" setup>
import { Application } from 'vue3-pixi'
import { NCard, NScrollbar } from 'naive-ui'
import { ref, toRefs, defineProps, computed } from 'vue'
import TimeComponent from '../TimeComponent.vue'
import {
  CrazyCountingHostEntitiesPayload,
  CrazyCountingResultPayload,
  FBCrazyCountingEntity,
  GameStateType,
  MiniGameIntroductionPayload,
  MiniGamePayloadType
} from '@/flatbuffers/messageClass'
import { type Player } from '@/stores/confettiStore'
import { type IntroductionData } from '@/components/introduction/Introduction.vue'
import Introduction from '@/components/introduction/Introduction.vue'

const size = computed(() => appSize.value / 10)

const props = defineProps<{
  width: number
  height: number
}>()

interface PosData {
  x: number
  y: number
}

interface ResultsPair {
  name: string
  guess: number
}

interface Result {
  correct: number
  results: ResultsPair[]
}

enum ViewState {
  None,
  Introduction,
  MiniGame,
  Results
}

const viewState = ref<ViewState>(ViewState.None)

const { width, height } = toRefs(props)

const appSize = computed(() => {
  return Math.min(width.value, height.value)
})

const interpolatePosition = (entity: FBCrazyCountingEntity): PosData => {
  return {
    x: entity.xPos() * (appSize.value - size.value),
    y: entity.yPos() * (appSize.value - size.value)
  }
}

// introduction
const intro = ref<IntroductionData>({
  title: '',
  description: ''
})
// game data
const entities = ref<PosData[]>([])
const timeLeft = ref<number>(0)
const submittedPlayers = ref<string[]>([])
// results
const results = ref<Result>({
  correct: 0,
  results: []
})

const update = (data: MiniGamePayloadType) => {
  switch (data.gamestatetype()) {
    case GameStateType.CrazyCountingHostEntities: {
      viewState.value = ViewState.MiniGame
      const hostEntitiesPayload: CrazyCountingHostEntitiesPayload = data.gamestatepayload(
        new CrazyCountingHostEntitiesPayload()
      )

      let localEntities: PosData[] = []
      for (let i = 0; i < hostEntitiesPayload.entitiesLength(); i++) {
        const entity = hostEntitiesPayload.entities(i)
        if (entity === null) continue
        localEntities.push(interpolatePosition(entity))
      }
      entities.value = localEntities

      timeLeft.value = Number(hostEntitiesPayload.timeLeft())

      let newSubmittedPlayers: string[] = []
      for (let i = 0; i < hostEntitiesPayload.submittedLength(); i++) {
        const submittedString = hostEntitiesPayload.submitted(i)
        if (submittedString === null) continue
        newSubmittedPlayers.push(submittedString)
      }
      submittedPlayers.value = newSubmittedPlayers

      return localEntities
    }
    case GameStateType.CrazyCountingResult: {
      viewState.value = ViewState.Results
      const resultsPayload: CrazyCountingResultPayload = data.gamestatepayload(new CrazyCountingResultPayload())

      let newSubmittedPlayers: ResultsPair[] = []
      for (let i = 0; i < resultsPayload.resultsLength(); i++) {
        const submittedString = resultsPayload.results(i)
        if (submittedString === null) continue
        newSubmittedPlayers.push({ name: submittedString.name() || '', guess: submittedString.guess()})
      }
      
      results.value = {
        correct: resultsPayload.correctAnswer(),
        results: newSubmittedPlayers
      }
      return results.value
    }
    case GameStateType.MiniGameIntroduction: {
      viewState.value = ViewState.Introduction
      const introPayload: MiniGameIntroductionPayload = data.gamestatepayload(new MiniGameIntroductionPayload())
      intro.value = {
        title: introPayload.name() || '',
        description: introPayload.instruction() || ''
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
    <Introduction :data="intro" />
  </div>
  <div v-else-if="viewState == ViewState.MiniGame" class="flex justify-stretch">
    <div class="mt-4 w-full h-full flex flex-col justify-center">
      <div class="mx-auto mb-4">
        <TimeComponent :timeLeft />
      </div>
      <p class="text-4xl w-full text-center text-white">Answers locked:</p>
      <n-scrollbar class="mt-4">
        <div
          class="mx-auto mb-4 w-4/5"
          v-for="(name, i) in submittedPlayers.slice().reverse()"
          :key="i"
        >
          <n-card>
            <p class="font-bold text-2xl w-full text-center overflow-ellipsis">
              {{ name }}
            </p>
          </n-card>
        </div>
      </n-scrollbar>
    </div>
    <div class="relative">
      <Application key="gameview" :width="appSize" :height="appSize" background-color="white">
        <sprite
          v-for="(entity, i) in entities"
          :position="entity"
          :width="size"
          :height="size"
          :key="i"
          texture="/assets/games/crazyCounting/partyhat.svg"
        />
      </Application>
    </div>
  </div>
  <div v-else-if="viewState == ViewState.Results">
    <div class="flex flex-col gap-4 w-full h-full">
      <div class="flex">
        <p class="text-9xl w-full text-center m-auto text-primary">
          {{ results.correct }}
        </p>
      </div>
      <p class="text-4xl w-full text-center text-white">Players guesses:</p>
      <n-scrollbar class="-mb-4">
        <div class="mx-auto mb-4 w-4/5" v-for="(player, i) in results.results" :key="i">
          <n-card>
            <div class="w-full inline-flex justify-between text-2xl px-4">
              <p class="inline-flex">
                <span class="font-bold ml-4 col-span-5">{{ player.name }}</span>
              </p>
              <p>
                <span class="font-bold">{{ player.guess }}</span>
              </p>
            </div>
          </n-card>
        </div>
      </n-scrollbar>
    </div>
  </div>
</template>
