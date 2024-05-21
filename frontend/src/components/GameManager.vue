<script lang="ts" setup>
import { MiniGamePayloadType } from '@/flatbuffers/messageClass'
import { useWebSocketStore } from '@/stores/confettiStore'
import { defineAsyncComponent, ref, watch, defineProps, onMounted, shallowRef } from 'vue'
import HostView from './crazyCounting/HostView.vue'

const props = defineProps<{
  isHost: boolean
}>()

const websocketStore = useWebSocketStore()
const gameName = ref('')

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

const gameViewComp = shallowRef(getComponent(gameName.value))
const gameViewRef = ref<InstanceType<typeof HostView>>()

const height = ref(0)
const width = ref(0)

onMounted(() => {
  const container = document.getElementById('container') as HTMLDivElement

  const resizeObserver = new ResizeObserver((entries) => {
    const { width: wid, height: hei } = entries[0].contentRect
    height.value = hei
    width.value = wid
  })

  resizeObserver.observe(container)

  const unsubscribe = websocketStore.subscribe((data: MiniGamePayloadType) => {
    // decide which minigame to show
    if (data instanceof MiniGamePayloadType) {
      // TODO: Make this dynamic from the server
      if (gameName.value === '') {
        gameName.value = 'crazyCounting'
      }

      if (gameViewRef.value?.update) gameViewRef.value?.update(data)
    }
  })

  return () => {
    resizeObserver.disconnect()
    unsubscribe
  }
})

watch(
  gameName,
  debounce((value: string) => {
    gameViewComp.value = getComponent(value)
  }, 500)
)
</script>
<template>
  <div
    id="container"
    class="p-2 bg-black backdrop-blur-xl bg-opacity-50 shadow-lg rounded-md w-full h-[97dvh] m-3"
  >
    <div class="w-full flex justify-center">
      <component :is="gameViewComp" ref="gameViewRef" v-if="width && height" :height :width />
    </div>
  </div>
</template>
