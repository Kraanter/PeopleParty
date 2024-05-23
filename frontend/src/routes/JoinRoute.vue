<script setup lang="ts">
import { computed, onMounted, watch, watchEffect } from 'vue'
import { NCard, NInput, NButton, NResult, NH1, NCollapseTransition } from 'naive-ui'
import { ref } from 'vue'
import { useRoute } from 'vue-router'
import { useWebSocketStore } from '@/stores/confettiStore'
import GameManager from '@/components/GameManager.vue'
import { debounce } from '@/util/funcs'
import PeoplePartyLogo from '@/components/PeoplePartyLogo.vue'
import { storeToRefs } from 'pinia'
import { useViewStore, ViewState } from '@/stores/viewStore'

const websocketStore = useWebSocketStore()
const { partyCode } = storeToRefs(websocketStore)

const viewStore = useViewStore()
const { viewState, viewData } = storeToRefs(viewStore)

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
    console.log('Joining party success: ', success)
    if (success) {
      partyCode.value = codeString.value
      joinPromise.value = undefined
      joined.value = true
      error.value = ''
    } else {
      joinPromise.value = undefined
      error.value = 'Failed to join party. Please check the code and try again.'
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
  () => codeString.value?.length === partyCodeLength && username.value.length > 3
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
    inputElements.value[index].select()
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
  if (username.value.length < 4) {
    error.value = 'Username must be at least 4 characters long.'
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
    <!-- ViewState === MiniGame -->
    <GameManager
      :data="viewData"
      :is-host="false"
      v-if="viewState === ViewState.MiniGame"
      class="row-span-3"
    />

    <!-- Joined message -->
    <n-card class="max-w-md m-auto row-start-2" v-if="viewState === ViewState.PartyPrep">
      <n-result
        status="success"
        title="Joined"
        :description="`Succesfully joined party: ${partyCode}!`"
      />
    </n-card>

    <!-- ViewState === None -->
    <template v-if="viewState === ViewState.None">
      <div class="w-full h-full">
        <PeoplePartyLogo />
      </div>
      <n-card class="max-w-md m-auto">
        <n-h1 class="mb-6 text-center">Join a Party!</n-h1>

        <div class="flex justify-between gap-4">
          <n-input
            v-bind:key="i"
            v-for="i in partyCodeLength"
            placeholder=""
            ref="inputElements"
            id="partyCode"
            size="large"
            :autofocus="i === 1"
            :disabled="joining"
            class="text-center flex items-center !text-3xl font-extrabold aspect-square bg-slate-300"
            :theme-overrides="{ caretColor: 'transparent' }"
            :value="code[i - 1]"
            pattern="\d*"
            :allow-input="onlyAllowNumber"
            @input="onChange(i - 1, $event.trim())"
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

      <!-- Redirect link to /host -->
      <router-link v-if="!(joining || joined)" to="/host" class="bottom-0 m-auto mb-10 underline">
        Host your own party!
      </router-link>
    </template>
  </div>
</template>
