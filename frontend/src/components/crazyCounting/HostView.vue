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
  data: MiniGamePayloadType
}>()

const entities = ref<FBCrazyCountingEntity[]>([])

const { width, height, data } = toRefs(props)

const appSize = computed(() => {
  return Math.min(width.value, height.value)
})

const interpolatePosition = (entity: FBCrazyCountingEntity) => {
  return {
    x: Math.abs(entity.xPos()) * (appSize.value - size.value),
    y: Math.abs(entity.yPos()) * (appSize.value - size.value)
  }
}

const proccessData = (data: MiniGamePayloadType) => {
  switch (data.gamestatetype()) {
    case GameStateType.CrazyCountingHostEntities: {
      const hostEntitiesPayload: CrazyCountingHostEntitiesPayload = data.gamestatepayload(
        new FBCrazyCountingEntity()
      )
      let localEntities: FBCrazyCountingEntity[] = []

      for (let i = 0; i < hostEntitiesPayload.entitiesLength(); i++) {
        const entity = hostEntitiesPayload.entities(i)
        localEntities.push(entity!)
      }
      entities.value = localEntities
    }
  }
}

watch(
  data,
  (newData) => {
    proccessData(newData)
  },
  { immediate: true }
)
</script>
<template>
  <div class="w-full flex justify-center">
    <Loader :resources="['/assets/games/crazyCounting/circle.svg']">
      <template #fallback="{ progress }">
        <text :x="120" :y="120" :anchor="0.5">
          <!-- TODO: Add a nice loading screen -->
          {{ `Loading... ${progress}` }}
        </text>
      </template>
      <Application :width="appSize" :height="appSize" background-color="white">
        <sprite
          v-for="(entity, i) in entities"
          :position="interpolatePosition(entity as FBCrazyCountingEntity)"
          :key="i"
          texture="/assets/games/crazyCounting/circle.svg"
        />
      </Application>
    </Loader>
  </div>
</template>
