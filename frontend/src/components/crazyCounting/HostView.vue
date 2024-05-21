<script lang="ts" setup>
import { Application, Loader, type LoadAsset } from 'vue3-pixi'
import { ref, toRefs, defineProps, watch, computed } from 'vue'
import {
  CrazyCountingHostEntitiesPayload,
  FBCrazyCountingEntity,
  GameStateType,
  type MiniGamePayloadType
} from '@/flatbuffers/messageClass'

const size = ref(100)

const props = defineProps<{
  width: number
  height: number
  data: PosData[]
}>()

interface PosData {
  x: number
  y: number
}

const entities = ref<PosData[]>([])

const { width, height, data } = toRefs(props)

const appSize = computed(() => {
  return Math.min(width.value, height.value)
})

const interpolatePosition = (entity: FBCrazyCountingEntity, appSize: number): PosData => {
  return {
    x: Math.abs(entity.xPos()) * (appSize - size.value),
    y: Math.abs(entity.yPos()) * (appSize - size.value)
  }
}

const proccessData = (data: MiniGamePayloadType) => {
  if (!data) return
  const size = appSize.value
  switch (data.gamestatetype()) {
    case GameStateType.CrazyCountingHostEntities: {
      const hostEntitiesPayload: CrazyCountingHostEntitiesPayload = data.gamestatepayload(
        new CrazyCountingHostEntitiesPayload()
      )
      let localEntities: PosData[] = []
      for (let i = 0; i < hostEntitiesPayload.entitiesLength(); i++) {
        const entity = hostEntitiesPayload.entities(i)
        if (entity === null) continue
        localEntities.push(interpolatePosition(entity, size))
      }
      entities.value = localEntities
    }
  }
}

// watch(
//   data,
//   (newData) => {
//     proccessData(newData)
//   },
//   { immediate: true }
// )
</script>
<template>
  <sprite
    v-for="(entity, i) in data"
    :position="entity"
    :key="i"
    texture="/assets/games/crazyCounting/circle.svg"
  />
</template>
