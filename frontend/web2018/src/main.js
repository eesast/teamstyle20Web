// The Vue build version to load with the `import` command
// (runtime-only or standalone) has been set in webpack.base.conf with an alias.
import Vue from 'vue'
import App from './App'
import router from './router'
import ElementUI from 'element-ui'
import 'element-ui/lib/theme-chalk/index.css'
import VueClipboard from 'vue-clipboard2'
import VueParticles from 'vue-particles'

import VueResource from 'vue-source'
Vue.use(VueResource)
Vue.use(VueParticles)
Vue.use(VueClipboard)
Vue.use(ElementUI)

Vue.config.productionTip = false
// setCookie("token",null)
// setCookie("username",null)
// setCookie("id",null)
//为什么要设置为空，这会在每次刷新的时候挂掉

function setCookie(cname,cvalue){
      document.cookie = cname+"="+cvalue+";path=/";
    }

/* eslint-disable no-new */
new Vue({
  el: '#app',
  router,
  components: { App },
  template: '<App/>'
})

