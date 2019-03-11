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
import error from '@/components/error.vue'
import emptyindex from '@/components/empty_index.vue'

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
      name: announcement,
      component: announcement
    },
    {
      path: '/empty',
      // name:'announcement',
      name: empty,
      component: empty
    },
    {
      path: '/login',
      name: login,
      component: login
    },
    {
      path: '/file',
      name: file,
      component: file
    },
    {
      path: '/team',
      name: team,
      component: team
    },
    {
      path: '/battle',
      name: battle,
      component: battle
    },
    {
      path: '/personal',
      name: personal,
      component: personal
    },
    {
      path: '/alter',
      name: alter,
      component: alter
    },
    {
      path: '/empty_index',
      name: emptyindex,
      component: emptyindex
    },
    {
      path: '*',
      name: error,
      component: error
    }
  ]
})
