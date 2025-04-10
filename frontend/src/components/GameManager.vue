<script lang="ts" setup>
import { MiniGamePayloadType } from '@/flatbuffers/messageClass'
import { defineAsyncComponent, ref, watch, defineProps, onMounted, shallowRef, toRefs } from 'vue'
import { debounce } from '@/util/funcs'
import PeoplePartyHeader from './PeoplePartyHeader.vue'
import { useWebSocketStore } from '@/stores/confettiStore';

const websocketStore = useWebSocketStore()

const props = defineProps<{
  data: MiniGamePayloadType
  isHost: boolean
}>()
const { data } = toRefs(props)

const gameName = ref('')

const componentName = props.isHost ? 'Host' : 'Player'

const getComponent = (name: string) => {
  if (name === '') {
    return defineAsyncComponent(() => import('./LoadingView.vue'))
  } else {
    return defineAsyncComponent(() => import(`./${name}/${componentName}View.vue`))
  }
}

const gameViewComp = shallowRef(getComponent(gameName.value))
const gameViewRef = ref()

const height = ref(0)
const width = ref(0)

onMounted(() => {
  const container = document.getElementById('container') as HTMLDivElement

  const resizeObserver = new ResizeObserver((entries) => {
    const { width: wid, height: hei } = entries[0].contentRect
    height.value = hei
    width.value = wid

    // Scroll the app into view
    container.scrollIntoView({ behavior: 'smooth' })
  })

  resizeObserver.observe(container)

  return () => {
    resizeObserver.disconnect()
  }
})

watch(data, (data: MiniGamePayloadType) => {
  // decide which minigame to show
  if (data instanceof MiniGamePayloadType) {
    if (gameName.value === '' || gameName.value !== data.minigame()) {
      gameName.value = data.minigame() || ''
    }

    if (gameViewRef.value?.update) gameViewRef.value?.update(data)
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
  <div class="flex flex-col h-full w-full">
    <PeoplePartyHeader v-if="isHost" />

    
    <div
      id="container"
      class="backdrop-blur-xl bg-opacity-50 shadow-lg rounded-xl w-full h-[97dvh] overflow-hidden"
    >
      <div v-if="websocketStore.isPaused" class="fixed h-full w-full justify-center items-center z-30">
        <div style="background-color: rgb(0 0 0 / .4)" class="fixed flex h-full w-full justify-center items-center z-30">
          <div style="background-color: rgb(0 0 0 / .65);" class="rounded-xl">
            <span class="text-6xl text-white m-3 flex">Game is paused</span>
          </div>
        </div>
        <div class="fixed h-full w-full backdrop-blur-xl opacity-60"></div>
      </div>
      <component v-if="width && height" :is="gameViewComp" ref="gameViewRef" :height :width />
    </div>
  </div>
</template>
