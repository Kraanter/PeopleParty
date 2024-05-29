<!-- eslint-disable vue/multi-word-component-names -->
<script setup lang="ts">
import { defineProps, toRefs } from 'vue'
import TimeComponent from '../TimeComponent.vue'

export type IntroductionData = {
  title: string;
  description: string;
  time_left: number;
}

const props = defineProps<{
  logoSVG: string;
  data: IntroductionData
}>()

const { data: introduction } = toRefs(props)

</script>
<template>
  <div class="flex flex-col gap-4 w-full h-full">
    <div class="w-full flex justify-center px-8 mt-4">
      <div>
        <TimeComponent :timeLeft="introduction.time_left" />
      </div>
    </div>
    <div class="flex justify-stretch mt-4 flex flex-col justify-center">
      <img :src="props.logoSVG" alt="logo" class="w-1/2 h-1/2 mx-auto" />
    </div>

    <div class="flex justify-stretch mt-4 w-full h-full flex flex-col justify-center">
      <p class="text-8xl w-full text-center text-white">{{ introduction.title }}</p>
      <p class="text-4xl w-full text-center text-white">{{ introduction.description }}</p>
    </div>
  </div>
</template>
