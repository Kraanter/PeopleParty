import './assets/main.css'

import { createApp } from 'vue'
import { createPinia } from 'pinia'

import App from './App.vue'
import router from './router'

// Make screen not sleep
navigator?.wakeLock?.request('screen')

// Handle mobile orientation and viewport issues
const handleOrientationAndViewport = () => {
  // Force portrait orientation if supported
  try {
    // @ts-ignore - orientation.lock is not in all TypeScript definitions but is supported
    if (screen?.orientation?.lock) {
      // @ts-ignore
      screen.orientation.lock('portrait').catch(() => {
        // Fallback: orientation lock not supported or failed
        console.log('Orientation lock not supported')
      })
    }
  } catch (error) {
    console.log('Orientation lock not available')
  }

  // Handle viewport height issues on mobile (especially iOS)
  const setViewportHeight = () => {
    const vh = window.innerHeight * 0.01
    document.documentElement.style.setProperty('--vh', `${vh}px`)
  }

  // Set viewport height initially
  setViewportHeight()

  // Update on resize/orientation change
  window.addEventListener('resize', () => {
    // Small delay to ensure dimensions are updated
    setTimeout(setViewportHeight, 100)
  })

  window.addEventListener('orientationchange', () => {
    // Longer delay for orientation changes
    setTimeout(() => {
      setViewportHeight()
      // Force a layout recalculation
      document.body.style.height = '100vh'
      document.body.style.height = 'calc(var(--vh, 1vh) * 100)'
    }, 300)
  })
}

// Initialize orientation and viewport handling (only on mobile devices)
if (/Mobi|Android/i.test(navigator.userAgent)) {
  handleOrientationAndViewport()
}

const app = createApp(App)

app.use(createPinia())
app.use(router)

const meta = document.createElement('meta')
meta.name = 'naive-ui-style'
document.head.appendChild(meta)

app.config.warnHandler = (msg, vm, trace) => {
    // Suppress only the "non-emits event listeners" warning
    if (msg.includes("Extraneous non-emits event listeners")) {
      return;
    }
    // Log other warnings normally
    console.warn(msg, trace);
  };

app.mount('#app')
