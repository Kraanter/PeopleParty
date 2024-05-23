<script lang="ts" setup>
import { Application } from 'vue3-pixi'
import { ref, toRefs, defineProps, computed, watch } from 'vue'
import TimeComponent from '../TimeComponent.vue'
import {
  CrazyCountingHostEntitiesPayload,
  FBCrazyCountingEntity,
  GameStateType,
  MiniGamePayloadType
} from '@/flatbuffers/messageClass'

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

const update = (data: MiniGamePayloadType) => {
  switch (data.gamestatetype()) {
    case GameStateType.CrazyCountingHostEntities: {
      const hostEntitiesPayload: CrazyCountingHostEntitiesPayload = data.gamestatepayload(
        new CrazyCountingHostEntitiesPayload()
      )
      let localEntities: PosData[] = []
      for (let i = 0; i < hostEntitiesPayload.entitiesLength(); i++) {
        const entity = hostEntitiesPayload.entities(i)
        console.log(entity?.xPos(), entity?.yPos())
        if (entity === null) continue
        localEntities.push(interpolatePosition(entity))
      }
      entities.value = localEntities
      timeLeft.value = Number(hostEntitiesPayload.timeLeft())
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
    <div class="mx-auto mt-4">
      <TimeComponent :timeLeft />
    </div>
    <Application key="gameview" :width="appSize" :height="appSize" background-color="white">
      <sprite
        v-for="(entity, i) in entities"
        :position="entity"
        :width="size"
        :height="size"
        :key="i"
        texture="/assets/games/crazyCounting/circle.svg"
      />
    </Application>
  </div>
</template>
