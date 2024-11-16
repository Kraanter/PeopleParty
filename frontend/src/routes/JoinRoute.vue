<script setup lang="ts">
import { computed, onMounted, watch, watchEffect } from 'vue'
import { NCard, NInput, NButton, NH1, NCollapseTransition } from 'naive-ui'
import { ref } from 'vue'
import { useRoute } from 'vue-router'
import { useWebSocketStore } from '@/stores/confettiStore'
import GameManager from '@/components/GameManager.vue'
import { debounce } from '@/util/funcs'
import PeoplePartyLogo from '@/components/PeoplePartyLogo.vue'
import Leaderboard from '@/components/leaderboard/Leaderboard.vue'
import { storeToRefs } from 'pinia'
import { useViewStore, ViewState } from '@/stores/viewStore'
import { useReleasesStore } from '@/stores/releasesStore'

const websocketStore = useWebSocketStore()
const { partyCode } = storeToRefs(websocketStore)

const viewStore = useViewStore()
const { viewState, viewData } = storeToRefs(viewStore)

const releasesStore = useReleasesStore()

const onlyAllowNumber = (value: string) => !value || /^\d+$/.test(value)

const code = ref<string[]>([])
const username = ref('')
const error = ref('')
const inputElements = ref<HTMLInputElement[]>([])
const nameInput = ref<HTMLInputElement>()
const joinPromise = ref<Promise<void>>()
const joined = ref(false)

const route = useRoute()

onMounted(() => {
  onChange(0, (route.query.code as string) ?? '')
  if (codeString.value.length === partyCodeLength) {
    nameInput.value?.focus()
  }

  const unsubscribe = websocketStore.subscribe((success: boolean) => {
    if (success) {
      partyCode.value = codeString.value
      joinPromise.value = undefined
      joined.value = true
      error.value = ''
    } else {
      joinPromise.value = undefined
      error.value = 'Failed to join party. Please check the code or try a different name.'
    }
  })

  // Unsubscribe when component is unmounted
  return unsubscribe
})

const getCodeString = () => code.value?.join('') ?? ''

const debounceCodeString = debounce(() => (codeString.value = getCodeString()), 50)

watch(() => getCodeString(), debounceCodeString)

const codeString = ref(getCodeString())
const joinable = computed(
  () => codeString.value?.length === partyCodeLength && username.value.length > 1
)

const joining = computed(() => !!joinPromise.value)
const hasError = computed(() => !!error.value)

const partyCodeLength = 4

watchEffect(() => {
  if (error.value) {
    setTimeout(() => {
      error.value = ''
    }, 10000)
  }
})

const changeSelected = (index: number) => {
  if (index > -1 && index < partyCodeLength) {
    inputElements.value[index]?.select()
  } else if (index === partyCodeLength) {
    setTimeout(() => {
      nameInput.value?.focus()
    }, 100)
  }
}

const onChange = (index: number, value: string) => {
  if (index < 0 || index >= partyCodeLength || !onlyAllowNumber(value[0])) return

  if (value) {
    if (index < partyCodeLength) {
      changeSelected(index + 1)
    }
  }

  const thisValue = value[0]

  // Set the index of the code to the value
  code.value[index] = thisValue

  if (value.length > 1) {
    onChange(index + 1, value.slice(1))
  }
}

const keyDown = (index: number, event: KeyboardEvent) => {
  if (event.key === 'Backspace') {
    if (code.value[index]) {
      code.value[index] = ''
    } else {
      if (index > 0) {
        code.value[index - 1] = ''
        changeSelected(index - 1)
      }
    }
  } else if (event.key === 'ArrowLeft') {
    changeSelected(index - 1)
  } else if (event.key === 'ArrowRight') {
    changeSelected(index + 1)
  } else if (event.key.length === 1 && onlyAllowNumber(event.key)) {
    code.value[index] = ''
  }
}

const join = () => {
  if (username.value.length < 2) {
    error.value = 'Username must be at least 2 characters long.'
  } else {
    if (joinPromise.value) return
    joinPromise.value = new Promise<void>(() => {
      websocketStore.join(codeString.value, username.value)
    })
  }
}
</script>
<template>
  <div class="grid grid-rows-3 grid-cols-1 justify-center">
    <div class="row-span-3" v-if="viewState == ViewState.Leaderboard">
      <Leaderboard :data="viewData" />
    </div>

    <!-- ViewState === MiniGame -->
    <GameManager
      :data="viewData"
      :is-host="false"
      v-if="viewState === ViewState.MiniGame"
      class="row-span-3"
    />

    <!-- Joined message -->
    <n-card class="max-w-md m-auto row-start-2" v-if="viewState === ViewState.PartyPrep">
      <div class="flex flex-col justify-center items-center gap-8">
        <div>
          <svg class="size-32" viewBox="0 0 32 32" fill="none" xmlns="http://www.w3.org/2000/svg">
            <path
              d="M5.04996 29.9L6.5216 29.5061L6.91647 28.6508L9.46694 28.7178L12.1194 28.0078L12.7168 27.0816L15.1987 27.1836L17.79 26.49C19.59 26.01 20.2 23.75 18.88 22.43L9.55996 13.1C8.23996 11.78 5.97996 12.38 5.49996 14.19L4.72734 17.0766L4.8355 19.4212L3.95471 19.9632L3.16482 22.9143L3.29717 25.1621L2.4337 25.6458L2.08996 26.93C1.59996 28.73 3.24996 30.38 5.04996 29.9Z"
              fill="#F9C23C"
            />
            <path
              d="M7.80079 25.0526C6.32047 23.6703 5.05958 22.006 3.94623 19.995L4.72357 17.0908C6.00204 19.8887 7.44018 21.98 9.16541 23.591C10.8281 25.1437 12.7976 26.2878 15.2358 27.1736L12.1066 28.0112C10.5047 27.2141 9.07999 26.2472 7.80079 25.0526Z"
              fill="#3F5FFF"
            />
            <path
              d="M4.28706 27.8862C3.57939 27.2244 2.97105 26.4832 2.43201 25.6523L3.17687 22.8694C3.87953 24.3596 4.66657 25.5034 5.65277 26.4256C6.63663 27.3457 7.8612 28.085 9.4909 28.7113L6.5021 29.5113C5.67933 29.0387 4.94637 28.5028 4.28706 27.8862Z"
              fill="#3F5FFF"
            />
            <path
              d="M19.11 25.69C20.51 24.29 18.77 20.29 15.23 16.75C11.69 13.21 7.69997 11.47 6.29997 12.87C4.89997 14.27 6.63997 18.27 10.18 21.81C13.72 25.35 17.72 27.08 19.11 25.69Z"
              fill="#6D4534"
            />
            <path
              d="M11.61 7.27004C12.85 9.59004 13.1 12.25 12.33 14.76C11.68 16.9 10.34 18.71 8.52001 19.96C8.00001 19.31 7.55001 18.66 7.17001 18.03C8.56001 17.08 9.58001 15.7 10.08 14.07C10.67 12.16 10.48 10.14 9.54001 8.38004C9.23001 7.80004 9.45001 7.09004 10.02 6.79004C10.6 6.48004 11.31 6.70004 11.61 7.27004Z"
              fill="#F70A8D"
            />
            <path
              d="M22.1498 16.7126C24.0405 15.219 26.4927 14.9149 28.6037 15.7022C29.1461 15.9045 29.7499 15.6288 29.9522 15.0863C30.1545 14.5439 29.8787 13.9402 29.3363 13.7379C26.5673 12.7052 23.3395 13.1011 20.8502 15.0675C20.3959 15.4263 20.3186 16.0855 20.6775 16.5398C21.0363 16.9941 21.6955 17.0715 22.1498 16.7126Z"
              fill="#F70A8D"
            />
            <path
              d="M6.00998 9.69003C6.65063 9.69003 7.16998 9.17068 7.16998 8.53003C7.16998 7.88938 6.65063 7.37003 6.00998 7.37003C5.36933 7.37003 4.84998 7.88938 4.84998 8.53003C4.84998 9.17068 5.36933 9.69003 6.00998 9.69003Z"
              fill="#F70A8D"
            />
            <path
              d="M26.13 8.53003C26.13 9.17068 25.6106 9.69003 24.97 9.69003C24.3293 9.69003 23.81 9.17068 23.81 8.53003C23.81 7.88938 24.3293 7.37003 24.97 7.37003C25.6106 7.37003 26.13 7.88938 26.13 8.53003Z"
              fill="#F70A8D"
            />
            <path
              d="M12.49 3.96C12.49 4.49019 12.0602 4.92 11.53 4.92C10.9998 4.92 10.57 4.49019 10.57 3.96C10.57 3.42981 10.9998 3 11.53 3C12.0602 3 12.49 3.42981 12.49 3.96Z"
              fill="#F9C23C"
            />
            <path
              d="M25.48 24.41C25.48 24.9457 25.0457 25.38 24.51 25.38C23.9743 25.38 23.54 24.9457 23.54 24.41C23.54 23.8743 23.9743 23.44 24.51 23.44C25.0457 23.44 25.48 23.8743 25.48 24.41Z"
              fill="#F9C23C"
            />
            <path
              d="M21.63 4.01999C21.47 3.87999 21.27 3.78999 21.05 3.77999C20.51 3.74999 20.05 4.15999 20.01 4.69999C19.94 5.79999 19.18 7.78999 18.89 8.45999C18.89 8.46499 18.8875 8.46999 18.885 8.47499C18.8825 8.47999 18.88 8.48499 18.88 8.48999C18.8264 8.62205 18.7691 8.75228 18.7081 8.88047C18.6061 8.75272 18.4934 8.63579 18.37 8.53C18.09 8.28 17.75 8.09 17.35 7.96L17.32 7.95C15.97 7.5 14.5 8.23 14.05 9.59C13.83 10.25 13.88 10.95 14.19 11.57C14.5 12.19 15.03 12.65 15.69 12.87C15.7608 12.8936 15.8327 12.9086 15.9044 12.9154C16.4096 13.0519 16.8986 13.0566 17.3693 12.9302C17.3408 13.0675 17.3077 13.2041 17.27 13.34C17.13 13.86 17.44 14.4 17.96 14.54C18.48 14.68 19.02 14.37 19.16 13.85C19.3853 13.0354 19.4829 12.2058 19.4529 11.3677C19.9732 10.7159 20.3888 9.9978 20.7 9.21999L20.7147 9.18435C20.9001 8.73574 21.8729 6.38206 21.97 4.80999C21.98 4.49999 21.85 4.20999 21.63 4.01999ZM16.75 9.81C17.0254 9.9018 17.3077 10.0563 17.4285 10.6861C16.936 11.1435 16.62 11.0933 16.34 11C16.3063 10.9888 16.2724 10.9795 16.2384 10.9721C16.1114 10.9124 16.0092 10.807 15.95 10.68C15.87 10.53 15.86 10.36 15.92 10.2C16.03 9.87 16.39 9.69 16.72 9.8L16.75 9.81Z"
              fill="#00A6ED"
            />
            <path
              d="M18.27 18.16C21.68 17.21 25.25 17.64 28.33 19.38C28.9 19.71 29.1 20.42 28.79 20.98C28.47 21.55 27.75 21.75 27.18 21.43C24.65 20.01 21.71 19.65 18.91 20.43C16.6 21.07 14.62 22.43 13.19 24.31C12.58 23.89 11.95 23.41 11.33 22.87C13.06 20.6 15.47 18.94 18.27 18.16Z"
              fill="#00A6ED"
            />
            <path
              d="M16.624 16.9853C16.624 17.6205 16.1091 18.1353 15.474 18.1353C14.8389 18.1353 14.324 17.6205 14.324 16.9853C14.324 16.3502 14.8389 15.8353 15.474 15.8353C16.1091 15.8353 16.624 16.3502 16.624 16.9853Z"
              fill="#00A6ED"
            />
          </svg>
        </div>
        <p class="text-6xl font-semibold">Joined!</p>
        <p class="text-xl">Succesfully joined party: {{ partyCode }}!</p>
      </div>
    </n-card>

    <!-- ViewState === None -->
    <template v-if="viewState === ViewState.None">
      <div class="w-full h-full">
        <PeoplePartyLogo />
      </div>
      <n-card class="max-w-md m-auto">
        <n-h1 class="mb-6 text-center">Join a Party!</n-h1>

        <div class="grid grid-rows-1 h-full grid-cols-4 justify-between gap-4 w-full">
          <input
            v-bind:key="i"
            v-for="i in partyCodeLength"
            type="number"
            :show-button="false"
            placeholder=""
            ref="inputElements"
            id="partyCode"
            :autofocus="i === 1"
            :disabled="joining"
            class="text-center text-3xl font-extrabold aspect-square bg-slate-300 cartoonShadow focus:bg-slate-100 rounded-md"
            :value="code[i - 1]"
            pattern="\d*"
            :allow-input="onlyAllowNumber"
            @wheel.prevent
            @input="onChange(i - 1, ($event as InputEvent).data?.trim() ?? '')"
            @keydown="keyDown(i - 1, $event)"
          />
        </div>

        <!-- Name input -->
        <n-collapse-transition class="mt-6" :show="codeString?.length === partyCodeLength">
          <n-input
            ref="nameInput"
            id="inputName"
            v-model:value="username"
            placeholder="Username"
            size="large"
            class="w-full"
            :builtin-theme-overrides="{ fontSizeLarge: '16px' }"
            @keydown.enter="join"
            :loading="joining"
            :disabled="joining"
          />
        </n-collapse-transition>

        <!-- Error message -->
        <n-collapse-transition :show="hasError">
          <p class="text-red-500 text-center mt-4">{{ error }}</p>
        </n-collapse-transition>

        <!-- Join button -->
        <n-collapse-transition :show="joinable" class="flex justify-center mt-6">
          <n-button
            id="joinButton"
            :disabled="!joinable || joining"
            type="primary"
            size="large"
            @click="join"
          >
            Join Party
          </n-button>
        </n-collapse-transition>
      </n-card>

      <div class="grid grid-cols-4 justify-between w-full bottom-0 m-auto mb-10">
        <p class="flex">{{ releasesStore.versionNumber }}</p>
        <div class="flex col-span-2 justify-center items-center">
          <router-link v-if="!(joining || joined)" to="/host" class="underline">
            Host your own party!
          </router-link>
        </div>
      </div>
    </template>
  </div>
</template>
