import './assets/main.css'

import { createApp } from 'vue'
import { createPinia } from 'pinia'

import App from './App.vue'
import router from './router'

// Make screen not sleep
navigator?.wakeLock?.request('screen')

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
