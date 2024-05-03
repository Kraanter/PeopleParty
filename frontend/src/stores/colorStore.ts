import { defineStore } from 'pinia'
import { computed } from 'vue'
import { useRoute } from 'vue-router'

type ColorPalette = {
  primary: HexValue
  secondary: HexValue
  accent: HexValue
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
      case 'host':
        return {
          primary: createHexValue('#6792D3'),
          secondary: createHexValue('#A0C8FF'),
          accent: createHexValue('#FFFFFF')
        }

      case 'join':
        return {
          primary: createHexValue('#FFCF00'),
          secondary: createHexValue('#FD8B7B'),
          accent: createHexValue('#FFFFFF')
        }

      default:
        return {
          primary: createHexValue('#6792d3'),
          secondary: createHexValue('#a0c8ff'),
          accent: createHexValue('#FFFFFF')
        }
    }
  })

  return { colorPalette }
})
