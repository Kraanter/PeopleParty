<script lang="ts" setup>
import { ref, watch } from 'vue'
import { Sprite } from 'pixi.js'
import { useGameCanvas, createEntityLayer, loadTexture } from '@/composables/pixi'
import type { CrazyCountingEntity } from './CrazyCountingModels'

const props = defineProps<{ entities: CrazyCountingEntity[] }>()

// Square world — the backend sends normalized [0,1] positions, no inherent pixel size, so this
// is purely a rendering-resolution choice. Kept square to match the original Math.min(w,h)
// canvas, but now letterboxed/centered instead of just shrunk into a corner.
const WORLD_SIZE = 600
const HAT_SIZE = WORLD_SIZE / 10
const HAT_TEXTURE = '/assets/games/crazyCounting/partyhat.svg'

interface IndexedEntity extends CrazyCountingEntity {
  index: number
}

const containerRef = ref<HTMLElement | null>(null)

const canvas = useGameCanvas({
  container: containerRef,
  worldSize: { width: WORLD_SIZE, height: WORLD_SIZE },
  backgroundColor: 0xffffff,
  backgroundAlpha: 1,
  render: () => drawScene()
})

const hatLayer = createEntityLayer<IndexedEntity, Sprite>(canvas.root, {
  key: (entity) => entity.index, // no stable id in the payload — array index is stable within one sync() call
  create: () => {
    const sprite = new Sprite()
    sprite.width = HAT_SIZE
    sprite.height = HAT_SIZE
    loadTexture(sprite, HAT_TEXTURE, canvas.invalidate)
    return sprite
  },
  update: (sprite, entity) => {
    // Default (0,0) anchor: reserve a full hat-size margin so the sprite's top-left corner
    // never pushes the hat past the world edge, same as the original interpolatePosition math.
    sprite.position.set(
      entity.x_pos * (WORLD_SIZE - HAT_SIZE),
      entity.y_pos * (WORLD_SIZE - HAT_SIZE)
    )
  }
})

function drawScene() {
  hatLayer.sync(props.entities.map((entity, index) => ({ ...entity, index })))
}

watch(
  () => props.entities,
  () => canvas.invalidate(),
  { immediate: true }
)
</script>

<template>
  <div ref="containerRef" class="w-full h-full flex items-center justify-center"></div>
</template>
