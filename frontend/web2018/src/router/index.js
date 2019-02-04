import Vue from 'vue'
import Router from 'vue-router'
import HelloWorld from '@/components/HelloWorld'
import announcement from '@/components/announcement'
import empty from '@/components/empty'

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
    }
  ]
})
