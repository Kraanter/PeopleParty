<script setup lang="ts">
import { NQrCode } from 'naive-ui'
import { useWebSocketStore } from '@/stores/confettiStore'
import { storeToRefs } from 'pinia'
import { computed, defineProps } from 'vue'

defineProps<{
  size?: number
}>()

const defaultSize = 250

const confettiStore = useWebSocketStore()
const { partyCode } = storeToRefs(confettiStore)

const partyURL = computed(() => `${window.location.origin}/join?code=${partyCode.value}`)
</script>
<template>
  <n-qr-code :size="size ?? defaultSize" style="padding: 0" :value="partyURL" />
</template>
