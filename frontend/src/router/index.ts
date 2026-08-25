import { isMobile } from '@/util/detectmobilebrowser'
import { createRouter, createWebHistory, type RouteRecordRaw } from 'vue-router'

const routes: RouteRecordRaw[] = [
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
    component: () => import('../routes/HostRoute.vue')
  },
  {
    path: '/join',
    name: 'join',
    component: () => import('../routes/JoinRoute.vue')
  },
  {
    path: '/info',
    name: 'info',
    component: () => import('../routes/InfoRoute.vue')
  },
  {
    path: '/releases',
    name: 'releases',
    component: () => import('../routes/ReleasesRoute.vue')
  },
  // Phase 1 PixiJS composable smoke test — dev-only, never present in a production build.
  ...(import.meta.env.DEV
    ? [
        {
          path: '/dev/pixi-playground',
          name: 'dev-pixi-playground',
          component: () => import('../routes/PixiPlaygroundRoute.vue')
        }
      ]
    : []),
  {
    path: '/:pathMatch(.*)*',
    redirect: '/'
  }
]

const router = createRouter({
  history: createWebHistory(import.meta.env.BASE_URL),
  routes
})

export default router
