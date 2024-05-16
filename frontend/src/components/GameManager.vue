<script lang="ts" setup>
import { MiniGamePayloadType } from '@/flatbuffers/messageClass';
import { useWebSocketStore } from '@/stores/confettiStore';
import { defineAsyncComponent, ref, watch, defineProps, onMounted } from 'vue'

const props = defineProps<{
  isHost: boolean
}>()

// Maybe get this list later from the backend
const availableGames = ['crazyCounting']

const websocketStore = useWebSocketStore()
const gameName = ref('')
const gameData = ref({} as MiniGamePayloadType)

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

// const hostData = ref<{ x: number; y: number }[]>([])

// for (let i = 0; i < 100; i++) {
//   hostData.value.push({ x: Math.random() * 2 - 1, y: Math.random() * 2 - 1 })
// }

// const gameData = ref({
//   amount: 10,
//   time: 100,
//   submitted: false
// })

onMounted(() => {
  // const interval = setInterval(() => {
  //   gameData.value.time--
  //   if (gameData.value.time <= 0) {
  //     gameData.value.time = 100
  //   }
  // }, 1000)

  const container = document.getElementById('container') as HTMLDivElement

  const resizeObserver = new ResizeObserver((entries) => {
    const { width: wid, height: hei } = entries[0].contentRect
    height.value = hei
    width.value = wid
  })

  resizeObserver.observe(container)

  // Handle the message
  const unsubscribe = websocketStore.subscribe((miniGamePayload: MiniGamePayloadType) => {
    // decide which minigame to show
    for (const miniGame of availableGames) {
      // miniGamePayload.gamestatetype().toString().toUpperCase().startsWith(miniGame.toUpperCase())
      // eslint-disable-next-line no-constant-condition
      if (true) {
        gameData.value = miniGamePayload
        gameName.value = miniGame
        break
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
</script>
<template>
  <div
    id="container"
    class="p-2 bg-black backdrop-blur-xl bg-opacity-50 shadow-lg rounded-md w-full h-[97dvh] m-3"
  >
    <component
      v-if="width && height"
      :is="game"
      :data="gameData"
      :height
      :width
    />
  </div>
</template>
