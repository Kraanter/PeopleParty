<script lang="ts" setup>
import { Application } from 'vue3-pixi'
import { ref, toRefs, defineProps, computed, watch } from 'vue'
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

interface HostData {
  timeLeft: number
  entities: PosData[]
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

const gameState = ref<HostData>({ timeLeft: 0, entities: [] })

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
      gameState.value.entities = localEntities
      gameState.value.timeLeft = Number(hostEntitiesPayload.timeLeft())
      return localEntities
    }
  }
  return []
}

watch(size, () => {
  console.log('Size changed', size.value)
})

defineExpose({
  update
})
</script>
<template>
  <Application key="gameview" :width="appSize" :height="appSize" background-color="white">
    <sprite
      v-for="(entity, i) in gameState.entities"
      :position="entity"
      :width="size"
      :height="size"
      :key="i"
      texture="/assets/games/crazyCounting/circle.svg"
    />
  </Application>
</template>
