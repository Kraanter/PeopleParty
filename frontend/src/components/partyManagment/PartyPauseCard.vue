<script setup lang="ts">
import { useWebSocketStore } from '@/stores/confettiStore'
import { storeToRefs } from 'pinia'
import { defineProps, defineEmits, ref } from 'vue'
import { NCard, NButton, NIcon } from 'naive-ui'
import PartyQrCode from './PartyQrCode.vue'

defineProps<{
  message: string
}>()

const emit = defineEmits(['click'])

const playerAmount = ref(10)

const confettiStore = useWebSocketStore()
const { partyCode } = storeToRefs(confettiStore)
</script>
<template>
  <n-card>
    <div class="grid grid-cols-3 grid-rows-1">
      <div class="flex flex-col justify-center items-center h-full w-full">
        <span class="text-lg mb-2">Join now at <span class="font-bold">peopleparty.nl</span></span>
        <span class="text-6xl font-semibold">Party Code:</span>
        <span class="text-6xl font-mono mt-2">{{ partyCode }}</span>
        <div class="pt-4">
          <PartyQrCode />
        </div>
      </div>
      <div class="w-full h-full flex flex-col justify-between items-center col-span-2">
        <div class="flex w-full justify-stretch gap-4">
          <div class="w-full">
            <n-button
              type="info"
              size="large"
              block
              circle
              :theme-overrides="{ iconMarginLarge: '0 90% 0' }"
            >
              <template #icon>
                <svg
                  xmlns="http://www.w3.org/2000/svg"
                  viewBox="0 0 24 24"
                  fill="currentColor"
                  class="w-8 h-8"
                >
                  <path
                    fill-rule="evenodd"
                    d="M7.5 6a4.5 4.5 0 1 1 9 0 4.5 4.5 0 0 1-9 0ZM3.751 20.105a8.25 8.25 0 0 1 16.498 0 .75.75 0 0 1-.437.695A18.683 18.683 0 0 1 12 22.5c-2.786 0-5.433-.608-7.812-1.7a.75.75 0 0 1-.437-.695Z"
                    clip-rule="evenodd"
                  />
                </svg>
              </template>
              {{ playerAmount }}
            </n-button>
          </div>
          <n-button type="primary" size="large" circle>
            <template #icon>
              <n-icon>
                <svg
                  xmlns="http://www.w3.org/2000/svg"
                  viewBox="0 0 24 24"
                  fill="currentColor"
                  class="w-full h-full"
                >
                  <path
                    fill-rule="evenodd"
                    d="M12 1.5a5.25 5.25 0 0 0-5.25 5.25v3a3 3 0 0 0-3 3v6.75a3 3 0 0 0 3 3h10.5a3 3 0 0 0 3-3v-6.75a3 3 0 0 0-3-3v-3c0-2.9-2.35-5.25-5.25-5.25Zm3.75 8.25v-3a3.75 3.75 0 1 0-7.5 0v3h7.5Z"
                    clip-rule="evenodd"
                  />
                </svg>
              </n-icon>
            </template>
          </n-button>
          <n-button type="primary" size="large" circle>
            <template #icon>
              <n-icon>
                <svg
                  xmlns="http://www.w3.org/2000/svg"
                  viewBox="0 0 24 24"
                  fill="currentColor"
                  class="w-8 h-8"
                >
                  <path
                    fill-rule="evenodd"
                    d="M11.078 2.25c-.917 0-1.699.663-1.85 1.567L9.05 4.889c-.02.12-.115.26-.297.348a7.493 7.493 0 0 0-.986.57c-.166.115-.334.126-.45.083L6.3 5.508a1.875 1.875 0 0 0-2.282.819l-.922 1.597a1.875 1.875 0 0 0 .432 2.385l.84.692c.095.078.17.229.154.43a7.598 7.598 0 0 0 0 1.139c.015.2-.059.352-.153.43l-.841.692a1.875 1.875 0 0 0-.432 2.385l.922 1.597a1.875 1.875 0 0 0 2.282.818l1.019-.382c.115-.043.283-.031.45.082.312.214.641.405.985.57.182.088.277.228.297.35l.178 1.071c.151.904.933 1.567 1.85 1.567h1.844c.916 0 1.699-.663 1.85-1.567l.178-1.072c.02-.12.114-.26.297-.349.344-.165.673-.356.985-.57.167-.114.335-.125.45-.082l1.02.382a1.875 1.875 0 0 0 2.28-.819l.923-1.597a1.875 1.875 0 0 0-.432-2.385l-.84-.692c-.095-.078-.17-.229-.154-.43a7.614 7.614 0 0 0 0-1.139c-.016-.2.059-.352.153-.43l.84-.692c.708-.582.891-1.59.433-2.385l-.922-1.597a1.875 1.875 0 0 0-2.282-.818l-1.02.382c-.114.043-.282.031-.449-.083a7.49 7.49 0 0 0-.985-.57c-.183-.087-.277-.227-.297-.348l-.179-1.072a1.875 1.875 0 0 0-1.85-1.567h-1.843ZM12 15.75a3.75 3.75 0 1 0 0-7.5 3.75 3.75 0 0 0 0 7.5Z"
                    clip-rule="evenodd"
                  />
                </svg>
              </n-icon>
            </template>
          </n-button>
        </div>
        <div class="text-center">
          <p class="text-8xl font-medium mb-6">Party Paused</p>
          <p class="text-4xl font-medium">Don't keep the pizza guy waiting</p>
        </div>
        <n-button @click="emit('click')" type="primary" size="large" block round>{{
          message
        }}</n-button>
      </div>
    </div>
  </n-card>
</template>
