<script lang="ts" setup>
import { Application } from 'vue3-pixi'
import { ref, toRefs, defineProps, computed } from 'vue'
import {
  CrazyCountingHostEntitiesPayload,
  FBCrazyCountingEntity,
  GameStateType,
  MiniGamePayloadType
} from '@/flatbuffers/messageClass'

const size = ref(100)

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

const gameState = ref<PosData[]>([])

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
      gameState.value = localEntities
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
  <Application key="gameview" :width="appSize" :height="appSize" background-color="white">
    <sprite
      v-for="(entity, i) in gameState"
      :position="entity"
      :key="i"
      texture="/assets/games/crazyCounting/circle.svg"
    />
  </Application>
</template>
