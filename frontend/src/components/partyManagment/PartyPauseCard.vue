<script setup lang="ts">
import { useWebSocketStore } from '@/stores/confettiStore'
import { storeToRefs } from 'pinia'
import { defineProps, defineEmits } from 'vue'
import { NCard, NQrCode, NButton } from 'naive-ui'

defineProps<{
  message: string
}>()

const emit = defineEmits(['click'])

const confettiStore = useWebSocketStore()
const { partyCode } = storeToRefs(confettiStore)

const generateURL = () => `${window.location.origin}/join?code=${partyCode.value}`
</script>
<template>
  <n-card>
    <div class="flex flex-col sm:flex-row">
      <div class="m-auto">
        <n-qr-code :size="250" style="padding: 0" :value="generateURL()" />
      </div>
      <div class="flex flex-col justify-between my-4 mx-8 gap-4">
        <p class="text-title text-center">Party code: {{ partyCode }}</p>
        <p class="text-body text-balance text-center">
          Share this QR code with your friends to join the game
        </p>
        <n-button @click="emit('click')" type="primary" size="large" block round>{{
          message
        }}</n-button>
      </div>
    </div>
  </n-card>
</template>
