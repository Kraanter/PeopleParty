<script lang="ts" setup>
import { ref } from 'vue'
import { NCard, NButton, NQrCode, NResult } from 'naive-ui'

const joined = ref(false)
const partyCode = ref('1234')

const generateURL = () => `${window.location.origin}/join?code=${partyCode.value}`
</script>
<template>
  <div class="flex justify-center items-center">
    <n-card class="lg:mx-44 md:max-w-2/3 2xl:mx-96 m-3" v-if="joined">
      <div class="flex flex-col sm:flex-row">
        <div class="m-auto">
          <n-qr-code :size="250" style="padding: 0" :value="generateURL()" />
        </div>
        <div class="flex flex-col justify-between my-4 mx-8 gap-4">
          <p class="text-title text-center">Party code: {{ partyCode }}</p>
          <p class="text-body text-balance text-center">
            Share this QR code with your friends to join the game
          </p>
          <n-button type="primary" size="large" block round>Start game</n-button>
        </div>
      </div>
    </n-card>
    <n-card class="max-w-md m-3" v-else>
      <n-result status="404" title="Host a Party!" size="huge">
        <template #footer>
          <n-button type="primary" size="large" block round @click="joined = true">Host</n-button>
        </template>
      </n-result>
    </n-card>

    <!-- Redirect link to /host -->
    <router-link to="/join" class="fixed bottom-8 mt-6 underline"> Join a party! </router-link>
  </div>
</template>
