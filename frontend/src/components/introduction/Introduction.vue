<!-- eslint-disable vue/multi-word-component-names -->
<script setup lang="ts">
import { defineProps, toRefs } from 'vue'

// todo: maybe add logo url or something
export type IntroductionData = {
  title: string;
  description: string;
}

const props = defineProps<{
  data: IntroductionData
}>()

const { data: introduction } = toRefs(props)

</script>
<template>
  <div class="flex justify-stretch mt-4 w-full h-full flex flex-col justify-center">
    <p class="text-8xl w-full text-center text-white">title: {{ introduction.title }}</p>
    <p class="text-4xl w-full text-center text-white">description: {{ introduction.description }}</p>
  </div>
</template>
