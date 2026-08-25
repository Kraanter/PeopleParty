import { fileURLToPath, URL } from 'node:url'

import { defineConfig } from 'vite'
import vue from '@vitejs/plugin-vue'
import VueDevTools from 'vite-plugin-vue-devtools'
import { compilerOptions, transformAssetUrls } from 'vue3-pixi'
import { visualizer } from 'rollup-plugin-visualizer'

// https://vitejs.dev/config/
export default defineConfig(({ mode }) => ({
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
    // Only runs on `npm run build:analyze` (vite build --mode analyze) so a normal
    // build/CI/Docker run stays untouched.
    ...(mode === 'analyze'
      ? [visualizer({ filename: 'dist/stats.html', gzipSize: true, brotliSize: true, open: true })]
      : []),
  ],
  resolve: {
    alias: {
      '@': fileURLToPath(new URL('./src', import.meta.url))
    }
  },
  server: {
    proxy: {
      "/confetti": {
        target: 'ws://localhost:7899/',
        rewrite: (path) => path.replace(/^\/confetti/, ''),
        ws: true,
      }
    }
  },
  build: {
    rollupOptions: {
      output: {
        manualChunks(id) {
          // Vite's own tiny dynamic-import runtime helper. Left unhandled, Rollup can
          // hoist its canonical definition into whichever named vendor chunk below is
          // also one of its consumers (pixi.js/vue3-pixi use dynamic import internally),
          // which then forces the entry chunk to eagerly import that entire vendor
          // chunk on every page load — defeating the point of splitting it out.
          if (id.includes('vite/preload-helper')) {
            return 'vite-runtime'
          }
          const normalizedId = id.replace(/\\/g, '/')
          if (
            normalizedId.includes('node_modules/pixi.js/') ||
            normalizedId.includes('node_modules/vue3-pixi/')
          ) {
            return 'pixi'
          }
          if (normalizedId.includes('node_modules/naive-ui/')) {
            return 'naive-ui'
          }
        }
      }
    }
  }
}))
