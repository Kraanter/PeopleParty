<script setup lang="ts">
import { computed, onMounted, watchEffect } from 'vue'
import { NCard, NInput, NButton, NResult, NH1, NCollapseTransition } from 'naive-ui'
import { ref } from 'vue'
import { useRoute } from 'vue-router'
import { useWebSocketStore } from '@/stores/confettiStore'
import GameManager from '@/components/GameManager.vue'

const websocketStore = useWebSocketStore()

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

const codeString = computed(() => code.value.join('') ?? '')
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
  event.preventDefault()
  if (event.key === 'Backspace') {
    if (code.value[index]) {
      code.value[index] = ''
    } else {
      if (index > 0) {
        changeSelected(index - 1)
      }
    }
  } else if (event.key === 'ArrowLeft') {
    changeSelected(index - 1)
  } else if (event.key === 'ArrowRight') {
    changeSelected(index + 1)
  } else if (event.key.length === 1) {
    onChange(index, event.key)
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
  <div class="flex justify-center items-center">
    <GameManager :is-host="false" v-if="joined" />
    <!-- TODO: Put this back the way it was -->
    <!-- <n-card class="max-w-md m-3" v-if="joined">
      <n-result
        status="success"
        title="Joined"
        :description="`Succesfully joined party: ${codeString}!`"
      />
    </n-card> -->
    <n-card class="max-w-md m-3" v-else>
      <n-h1 class="mb-6 text-center">Join a Party!</n-h1>

      <div class="flex justify-between gap-4">
        <n-input
          v-bind:key="i"
          v-for="i in partyCodeLength"
          placeholder=""
          :allow-input="onlyAllowNumber"
          ref="inputElements"
          id="partyCode"
          size="large"
          :autofocus="i === 1"
          :disabled="joining"
          class="text-center flex items-center !text-3xl font-extrabold aspect-square"
          :theme-overrides="{ caretColor: 'transparent', color: 'transparent' }"
          v-model:value="code[i - 1]"
          @keydown="keyDown(i - 1, $event)"
        />
        <!-- @input="onChange(i - 1, $event)" -->
      </div>

      <!-- Name input -->
      <n-collapse-transition class="mt-6" :show="codeString?.length === partyCodeLength">
        <n-input
          ref="nameInput"
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
        <n-button :disabled="!joinable || joining" type="primary" size="large" @click="join">
          Join Party
        </n-button>
      </n-collapse-transition>
    </n-card>

    <!-- Redirect link to /host -->
    <router-link v-if="!(joining || joined)" to="/host" class="fixed bottom-8 mt-6 underline">
      Host your own party!
    </router-link>
  </div>
</template>
