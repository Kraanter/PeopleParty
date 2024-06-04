<script lang="ts" setup>
import { MiniGamePayloadType } from '@/flatbuffers/messageClass'
import { defineAsyncComponent, ref, watch, defineProps, onMounted, shallowRef, toRefs } from 'vue'
import PeoplePartyHeader from './PeoplePartyHeader.vue'
import type { HostPayload, BailedPlayer } from './buisinessBailout/HostView.vue'

const props = defineProps<{
  data: MiniGamePayloadType
  isHost: boolean
}>()
const { data } = toRefs(props)

const gameName = ref('buisinessBailout')

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

const num = ref(0)
const value = ref(0)
const bailedPlayers = ref<BailedPlayer[]>([])
onMounted(() => {
  const container = document.getElementById('container') as HTMLDivElement

  const resizeObserver = new ResizeObserver((entries) => {
    const { width: wid, height: hei } = entries[0].contentRect
    height.value = hei
    width.value = wid
  })

  setInterval(() => {
    // value.value = (Math.sin(num.value / 10) + 1) * 100
    value.value = Math.pow(num.value / 10, 2)
    if (Math.random() > 0.95)
      bailedPlayers.value = [
        ...bailedPlayers.value,
        {
          name: 'Player ' + Math.floor(Math.random() * 100),
          time: num.value * 100
        }
      ]
    const newData: HostPayload = {
      time: num.value++ * 100,
      value: value.value,
      bailed_players: bailedPlayers.value
    }
    gameViewRef.value?.update(JSON.stringify(newData))
  }, 100)

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
      class="bg-black backdrop-blur-xl bg-opacity-50 shadow-lg rounded-xl w-full h-[97dvh] overflow-hidden"
    >
      <component v-if="width && height" :is="gameViewComp" ref="gameViewRef" :height :width />
    </div>
  </div>
</template>
