import Vue from 'vue'
import Router from 'vue-router'
import announcement from '@/components/announcement'
import empty from '@/components/empty'
import HelloWorld from '@/components/Index.vue'
import login from '@/components/Login.vue'
import file from '@/components/file.vue'
import team from '@/components/team.vue'
import battle from '@/components/battle.vue'
import personal from '@/components/personal.vue'
import alter from '@/components/alter.vue'

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
    },
    {
      path: '/file',
      component: file
    },
    {
      path: '/team',
      component: team
    },
    {
      path: '/battle',
      component: battle
    },
    {
      path: '/personal',
      component: personal
    },
    {
      path: '/alter',
      name: alter,
      component: alter
    }
  ]
})
