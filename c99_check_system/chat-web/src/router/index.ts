import { createRouter, createWebHistory } from 'vue-router'
import { getAdminToken } from '@/api/admin'

const router = createRouter({
  history: createWebHistory(import.meta.env.BASE_URL),
  routes: [
    {
      path: '/',
      name: 'chat',
      component: () => import('@/views/ChatView.vue'),
    },
    {
      path: '/admin/login',
      name: 'admin-login',
      component: () => import('@/views/AdminLogin.vue'),
    },
    {
      path: '/admin',
      name: 'admin',
      component: () => import('@/views/AdminDashboard.vue'),
      beforeEnter: (_to, _from, next) => {
        if (!getAdminToken()) {
          next({ name: 'admin-login' })
        } else {
          next()
        }
      },
    },
  ],
})

export default router
