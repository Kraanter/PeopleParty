<script lang="ts" setup>
import { NButton } from 'naive-ui'
import { MiniGamePayloadType } from '@/flatbuffers/messageClass'
import { useWebSocketStore } from '@/stores/confettiStore'
import { defineAsyncComponent, ref, watch, defineProps, onMounted, shallowRef } from 'vue'
import PeoplePartyLogo from './PeoplePartyLogo.vue'
import { storeToRefs } from 'pinia'

const props = defineProps<{
  isHost: boolean
}>()

const websocketStore = useWebSocketStore()
const { partyCode } = storeToRefs(websocketStore)
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

  const unsubscribe = websocketStore.subscribe((data: MiniGamePayloadType) => {
    // decide which minigame to show
    if (data instanceof MiniGamePayloadType) {
      if (gameName.value === '' || gameName.value !== data.minigame()) {
        gameName.value = data.minigame() || ''
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
  <div class="flex flex-col h-full w-full">
    <div
      v-if="isHost"
      class="w-full bg-white grid grid-cols-3 grid-rows-1 rounded-lg mt-2 mb-8 px-4 max-h-20 h-full text-2xl justify-center items-center cartoonShadow"
    >
      <p>Join now at <span class="font-bold">peopleparty.nl</span></p>
      <div class="w-full row-span-3 -mt-4 h-28 my-auto">
        <PeoplePartyLogo />
      </div>
      <p class="text-right mr-4">
        Party Code: <span class="font-extrabold mr-4">{{ partyCode }}</span>
        <n-button type="primary" class="w-full h-full" circle>
          <svg
            xmlns="http://www.w3.org/2000/svg"
            viewBox="0 0 24 24"
            fill="currentColor"
            class="w-6 h-6"
          >
            <path
              fill-rule="evenodd"
              d="M6.75 5.25a.75.75 0 0 1 .75-.75H9a.75.75 0 0 1 .75.75v13.5a.75.75 0 0 1-.75.75H7.5a.75.75 0 0 1-.75-.75V5.25Zm7.5 0A.75.75 0 0 1 15 4.5h1.5a.75.75 0 0 1 .75.75v13.5a.75.75 0 0 1-.75.75H15a.75.75 0 0 1-.75-.75V5.25Z"
              clip-rule="evenodd"
            />
          </svg>
        </n-button>
      </p>
    </div>

    <div
      id="container"
      class="bg-black backdrop-blur-xl bg-opacity-50 shadow-lg rounded-md w-full h-[97dvh]"
    >
      <component v-if="width && height" :is="gameViewComp" ref="gameViewRef" :height :width />
    </div>
  </div>
</template>
