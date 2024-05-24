<script lang="ts" setup>
import { Application } from 'vue3-pixi'
import { NCard, NScrollbar } from 'naive-ui'
import { ref, toRefs, defineProps, computed } from 'vue'
import TimeComponent from '../TimeComponent.vue'
import {
  CrazyCountingHostEntitiesPayload,
  FBCrazyCountingEntity,
  GameStateType,
  MiniGamePayloadType
} from '@/flatbuffers/messageClass'
import { type Player } from '@/stores/confettiStore'

const size = computed(() => appSize.value / 10)

const props = defineProps<{
  width: number
  height: number
}>()

interface PosData {
  x: number
  y: number
}

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

const entities = ref<PosData[]>([])
const timeLeft = ref<number>(0)
const submittedPlayers = ref<string[]>([])

const update = (data: MiniGamePayloadType) => {
  switch (data.gamestatetype()) {
    case GameStateType.CrazyCountingHostEntities: {
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
  }
  return []
}

defineExpose({
  update
})
</script>
<template>
  <div class="flex justify-stretch">
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
      <!-- fixme: until propper endminigame screen is there -->
      <div v-if="timeLeft <= 100" class="absolute w-full h-full top-0 left-0 flex backdrop-blur-xl">
        <p class="text-9xl w-full text-center m-auto text-primary">
          {{ entities.length }}
        </p>
      </div>
    </div>
  </div>
</template>
