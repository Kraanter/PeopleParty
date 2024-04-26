<script setup lang="ts">
import { computed, onMounted, watchEffect } from 'vue'
import { NCard, NInput, NButton, NResult, NH1, NCollapseTransition } from 'naive-ui'
import { ref } from 'vue'
import { useRoute } from 'vue-router'

const onlyAllowNumber = (value: string) => !value || /^\d+$/.test(value)

const code = ref<string[]>([])
const username = ref('')
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
})

const codeString = computed(() => code.value.join('') ?? '')
const joinable = computed(
    () => codeString.value?.length === partyCodeLength && username.value.length > 3
)
const joining = computed(() => !!joinPromise.value)

const partyCodeLength = 4

watchEffect(() => {
    if (codeString.value.length === partyCodeLength) {
        nameInput.value?.focus()
    }
})

const changeSelected = (index: number) => {
    if (index > -1 && index < partyCodeLength) {
        inputElements.value[index].select()
    }
}

const onChange = (index: number, value: string) => {
    if (index < 0 || index >= partyCodeLength) return

    if (value) {
        if (index < partyCodeLength - 1) {
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
                changeSelected(index - 1)
            }
        }
    }
}

const join = () => {
    joinPromise.value = new Promise<void>((resolve) => {
        setTimeout(() => {
            console.log('Joined party with code:', codeString.value, 'and username:', username.value)
            joinPromise.value = undefined
            joined.value = true
            resolve()
        }, 1000)
    })
}
</script>
<template>
    <div class="flex justify-center items-center">
        <n-card class="max-w-md m-3" v-if="joined">
            <n-result status="success" title="Joined" :description="`Succesfully joined party: ${codeString}!`" />
        </n-card>
        <n-card class="max-w-md m-3" v-else>
            <n-h1 class="mb-6 text-center">Join a Party!</n-h1>

            <div class="flex justify-between gap-4">
                <n-input v-bind:key="i" v-for="i in partyCodeLength" placeholder="" :allow-input="onlyAllowNumber"
                    ref="inputElements" id="partyCode" size="large" :autofocus="i === 1" :disabled="joining"
                    class="text-center flex items-center !text-3xl font-extrabold aspect-square"
                    v-model:value="code[i - 1]" @input="onChange(i - 1, $event)" @keydown="keyDown(i - 1, $event)" />
            </div>

            <!-- Name input -->
            <n-collapse-transition class="mt-6" :show="codeString?.length === partyCodeLength">
                <n-input ref="nameInput" v-model:value="username" placeholder="Username" size="large" class="w-full"
                    @keydown.enter="join" :loading="joining" :disabled="joining" />
            </n-collapse-transition>

            <!-- Join button -->
            <n-collapse-transition :show="joinable" class="flex justify-center mt-6">
                <n-button :disabled="!joinable || joining" type="primary" size="large" @click="join">
                    Join Party
                </n-button>
            </n-collapse-transition>
        </n-card>

        <!-- Redirect link to /host -->
        <router-link to="/host" class="fixed bottom-8 mt-6 underline">
            Host your own party!
        </router-link>
    </div>
</template>
