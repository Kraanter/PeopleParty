import './assets/main.css'

import { createApp } from 'vue'
import { createPinia } from 'pinia'

import App from './App.vue'
import router from './router'

const app = createApp(App)

app.use(createPinia())
app.use(router)

const meta = document.createElement('meta')
meta.name = 'naive-ui-style'
document.head.appendChild(meta)

app.mount('#app')
