<script lang="ts" setup>
import { MiniGamePayloadType } from '@/flatbuffers/messageClass'
import { defineAsyncComponent, ref, watch, defineProps, onMounted, shallowRef, toRefs } from 'vue'
import PeoplePartyHeader from './PeoplePartyHeader.vue'

const props = defineProps<{
  data: MiniGamePayloadType
  isHost: boolean
}>()
const { data } = toRefs(props)

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
const gameViewRef = ref()

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
      <component v-if="width && height" :is="gameViewComp" ref="gameViewRef" :height :width />
    </div>
  </div>
</template>
