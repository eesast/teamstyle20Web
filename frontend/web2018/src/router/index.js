import Vue from 'vue'
import Router from 'vue-router'
import HelloWorld from '@/components/Index.vue'
import login from '@/components/Login.vue'

Vue.use(Router)

export default new Router({
  routes: [
    {
      path: '/',
      name: 'HelloWorld',
      component: HelloWorld
    },
    {
      path: '/login',
      component: login
    }
  ]
})
