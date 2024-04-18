import { fileURLToPath, URL } from 'node:url'

import { defineConfig } from 'vite'
import vue from '@vitejs/plugin-vue'
import VueDevTools from 'vite-plugin-vue-devtools'
import { compilerOptions, transformAssetUrls } from 'vue3-pixi'

// https://vitejs.dev/config/
export default defineConfig({
  plugins: [
    vue({
      template: {
        // support for custom elements and remove the unknown element warnings
        compilerOptions,
        // support for asset url conversion
        transformAssetUrls,
      },
    }),
    VueDevTools(),
  ],
  resolve: {
    alias: {
      '@': fileURLToPath(new URL('./src', import.meta.url))
    }
  },
  server: {
    proxy: {
      "/confetti": {
        target: 'ws://localhost:7789',
        rewrite: (path) => path.replace(/^\/confetti/, ''),
        ws: true,
      }
    }
  }
})
