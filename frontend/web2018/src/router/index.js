import Vue from 'vue'
import Router from 'vue-router'
import announcement from '@/components/announcement'
import empty from '@/components/empty'
import HelloWorld from '@/components/Index.vue'
import login from '@/components/Login.vue'

Vue.use(Router)

export default new Router({
  mode: 'history',
  routes: [
    {
      path: '/',
      name: 'HelloWorld',
      component: HelloWorld
    },
    {
      path: '/announcement',
      // name:'announcement',
      component: announcement
    },
    {
      path: '/empty',
      // name:'announcement',
      component: empty
    },
    {
      path: '/login',
      component: login
    }
  ]
})
