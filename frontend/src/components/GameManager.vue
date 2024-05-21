<script lang="ts" setup>
import { Application, Loader, type LoadAsset } from 'vue3-pixi'
import {
  CrazyCountingHostEntitiesPayload,
  FBCrazyCountingEntity,
  GameStateType,
  MiniGamePayloadType
} from '@/flatbuffers/messageClass'
import { useWebSocketStore } from '@/stores/confettiStore'
import { defineAsyncComponent, ref, watch, defineProps, onMounted, computed } from 'vue'
import HostView from './crazyCounting/HostView.vue'

const props = defineProps<{
  isHost: boolean
}>()

// Maybe get this list later from the backend
const availableGames = ['crazyCounting']

const websocketStore = useWebSocketStore()
const gameName = ref('')
const gameData = ref<{ x: number; y: number }[]>()

const debounce = (func: Function, wait: number) => {
  let timeout: number | undefined
  return (...args: any[]) => {
    clearTimeout(timeout)
    timeout = setTimeout(() => {
      func(...args)
    }, wait)
  }
}

const componentName = props.isHost ? 'Host' : 'Player'

const getComponent = (name: string) => {
  if (name === '') {
    return defineAsyncComponent(() => import('./LoadingView.vue'))
  } else {
    return defineAsyncComponent(() => import(`./${name}/${componentName}View.vue`))
  }
}

const game = ref(getComponent(gameName.value))

const height = ref(0)
const width = ref(0)

const size = 100

const interpolatePosition = (entity: FBCrazyCountingEntity): { x: number; y: number } => {
  return {
    x: Math.abs(entity.xPos()) * (appSize.value - size),
    y: Math.abs(entity.yPos()) * (appSize.value - size)
  }
}

onMounted(() => {
  const container = document.getElementById('container') as HTMLDivElement

  const resizeObserver = new ResizeObserver((entries) => {
    const { width: wid, height: hei } = entries[0].contentRect
    height.value = hei
    width.value = wid
  })

  resizeObserver.observe(container)

  const next = ref(true)

  const unsubscribe = websocketStore.subscribe((data: MiniGamePayloadType) => {
    // decide which minigame to show
    if (data instanceof MiniGamePayloadType) {
      for (const miniGame of availableGames) {
        // fixme: decide which game to show depening on a property in the payload
        // miniGamePayload.gamestatetype().toString().toUpperCase().startsWith(miniGame.toUpperCase())
        // eslint-disable-next-line no-constant-condition
        if (next.value) {
          switch (data.gamestatetype()) {
            case GameStateType.CrazyCountingHostEntities: {
              const hostEntitiesPayload: CrazyCountingHostEntitiesPayload = data.gamestatepayload(
                new CrazyCountingHostEntitiesPayload()
              )
              let localEntities: { x: number; y: number }[] = []
              for (let i = 0; i < hostEntitiesPayload.entitiesLength(); i++) {
                const entity = hostEntitiesPayload.entities(i)
                if (entity === null) continue
                localEntities.push(interpolatePosition(entity))
              }
              gameData.value = localEntities
            }
          }
          if (gameName.value !== miniGame) gameName.value = miniGame
          break
        }
      }
    }
  })

  return () => {
    //clearInterval(interval)
    resizeObserver.disconnect()
    unsubscribe
  }
})

watch(
  gameName,
  debounce((value: string) => {
    game.value = getComponent(value)
  }, 500)
)

const appSize = computed(() => {
  return Math.min(width.value, height.value)
})
</script>
<template>
  <div
    id="container"
    class="p-2 bg-black backdrop-blur-xl bg-opacity-50 shadow-lg rounded-md w-full h-[97dvh] m-3"
  >
    <div class="w-full flex justify-center">
      <Loader :resources="['/assets/games/crazyCounting/circle.svg']">
        <template #fallback="{ progress }">
          <text :x="120" :y="120" :anchor="0.5">
            <!-- TODO: Add a nice loading screen -->
            {{ `Loading... ${progress}` }}
          </text>
        </template>
        <Application key="gameview" :width="appSize" :height="appSize" background-color="white">
          <HostView v-if="width && height && gameData" :data="gameData" :height :width />
        </Application>
      </Loader>
    </div>
  </div>
</template>
