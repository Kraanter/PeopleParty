import { isMobile } from '@/util/detectmobilebrowser'
import { createRouter, createWebHistory } from 'vue-router'

const router = createRouter({
  history: createWebHistory(import.meta.env.BASE_URL),
  routes: [
    {
      path: '/',
      redirect(to) {
        // If user is on mobile, redirect to join page
        if (isMobile()) {
          to.name = 'join'
          return to
        }

        // Otherwise, redirect to host page
        to.name = 'host'
        return to
      }
    },
    {
      path: '/host',
      name: 'host',
      component: () => import('../views/HostView.vue')
    },
    {
      path: '/join',
      name: 'join',
      component: () => import('../views/JoinView.vue')
    }
  ]
})

export default router
