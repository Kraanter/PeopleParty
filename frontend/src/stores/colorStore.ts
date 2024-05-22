import { defineStore } from 'pinia'
import { computed, watch } from 'vue'
import { useRoute } from 'vue-router'

type ColorPalette = {
  primary: ColorRange
  secondary: ColorRange
  text: HexValue
}

type ColorRange = {
  light: HexValue
  base: HexValue
  dark: HexValue
}

type HexValue = {
  string: string
  number: number
}

export const useColorStore = defineStore('color', () => {
  const route = useRoute()

  const hexToNumber = (hex: string): number => {
    return parseInt(hex.replace('#', ''), 16)
  }

  const createHexValue = (hex: string): HexValue => {
    return {
      string: hex,
      number: hexToNumber(hex)
    }
  }

  const colorPalette = computed<ColorPalette>(() => {
    switch (route.name) {
      default:
        return getCSSColorPallette()
    }
  })

  function getCSSProperty(name: string) {
    return getComputedStyle(document.documentElement).getPropertyValue(name)
  }

  function getCSSColorPallette(): ColorPalette {
    return {
      primary: {
        base: createHexValue(getCSSProperty('--color-primary')),
        dark: createHexValue(getCSSProperty('--color-primary-dark')),
        light: createHexValue(getCSSProperty('--color-primary-light'))
      },
      secondary: {
        base: createHexValue(getCSSProperty('--color-secondary')),
        dark: createHexValue(getCSSProperty('--color-secondary-dark')),
        light: createHexValue(getCSSProperty('--color-secondary-light'))
      },
      text: createHexValue(getCSSProperty('--color-text'))
    }
  }

  watch(colorPalette, () => {
    // Set css variables
    document.documentElement.style.setProperty(
      '--color-primary',
      colorPalette.value.primary.base.string
    )
    document.documentElement.style.setProperty(
      '--color-primary-dark',
      colorPalette.value.primary.dark.string
    )
    document.documentElement.style.setProperty(
      '--color-primary-light',
      colorPalette.value.primary.light.string
    )
    document.documentElement.style.setProperty(
      '--color-secondary',
      colorPalette.value.secondary.base.string
    )
    document.documentElement.style.setProperty(
      '--color-secondary-dark',
      colorPalette.value.secondary.dark.string
    )
    document.documentElement.style.setProperty(
      '--color-secondary-light',
      colorPalette.value.secondary.light.string
    )
    document.documentElement.style.setProperty('--color-text', colorPalette.value.text.string)
  })

  return { colorPalette }
})
