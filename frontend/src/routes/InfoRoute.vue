<script lang="ts" setup>
import { isMobile } from '@/util/detectmobilebrowser'
import { useViewStore } from '@/stores/viewStore'
import { storeToRefs } from 'pinia'

const viewStore = useViewStore()
const { versionNumber } = storeToRefs(viewStore)
</script>
<template>
  <div v-if="isMobile()">
    <div class="h-full w-full grid grid-rows-8 grid-cols-1 justify-center">
      <div class="row-start-1 row-span-7 max-h contain-content">
        <img class="h-full w-full" src="/assets/info-phone.svg" />
      </div>
      <div class="m-auto row-start-8 justify-center text-center">
        <router-link to="/join" class="underline mr-4 text-2xl">Join a party!</router-link>
        <p class="mt-8">{{ versionNumber }}</p>
      </div>
    </div>
  </div>
  <div v-else>
    <div class="w-full h-full grid grid-rows-8 grid-cols-1 justify-center">
      <div class="row-start-1 row-span-7 max-h contain-content">
        <img class="h-full w-full" src="/assets/info-desktop.svg" />
      </div>
      <div class="row-start-8 grid grid-cols-3 justify-center text-center">
        <p class="justify-start text-start mt-auto ml-2 mb-2">{{ versionNumber }}</p>
        <div class="flex mt-4 justify-between w-full">
          <router-link to="/join" class="underline text-4xl">Join a party!</router-link>
          <router-link to="/host" class="underline text-4xl">Host a party!</router-link>
        </div>
      </div>
    </div>
  </div>
</template>
