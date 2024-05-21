<script setup lang="ts">
import { RouterView } from 'vue-router'
import Background from '@/components/BackgroundComponent.vue'
import { NConfigProvider, NModalProvider, type GlobalThemeOverrides } from 'naive-ui'
import { useColorStore } from './stores/colorStore'
import { storeToRefs } from 'pinia'

const colorStore = useColorStore()
const { colorPalette } = storeToRefs(colorStore)

const themeOverrides: GlobalThemeOverrides = {
  common: {
    primaryColor: colorPalette.value.primary.base.string,
    primaryColorHover: colorPalette.value.primary.light.string,
    primaryColorPressed: colorPalette.value.primary.light.string,
    primaryColorSuppl: colorPalette.value.primary.dark.string,
    textColorBase: colorPalette.value.text.string
  },
  Card: {
    boxShadow: '0.5rem 0.5rem 0 #000',
    borderRadius: '2rem'
  },
  Button: {
    iconSizeLarge: '1.7rem',
    borderRadiusLarge: '2rem'
  }
}
</script>
<template>
  <n-config-provider :theme-overrides="themeOverrides">
    <n-modal-provider>
      <main class="relative w-screen min-h-screen h-screen select-none font-kanit">
        <RouterView class="absolute z-10 w-full h-full p-2 sm:p-4" />

        <Background class="absolute w-full h-full top-0 left-0"></Background>
      </main>
    </n-modal-provider>
  </n-config-provider>
</template>
