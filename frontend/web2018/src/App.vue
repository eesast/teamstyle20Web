<template>
  <div id="app">
    <navbar v-if="$route.name.name !== 'error'"></navbar>
    <router-view/>
    <foot v-if="$route.name.name !== 'error'"></foot>
  </div>
</template>

<script>
import navbar from './components/nav'
import foot from './components/foot'
export default {
  name: 'App',
  components:{navbar, foot},
   watch:{
  $route(to,from){
    console.log(to.path);
    if(to.path=="/")
    {
      // 跳转到首页
      window.location="http://teamstyle.eesast.com";
       
    }
    console.log(getCookie('token'));
    if(getCookie('token')!=null)
    {
      navbar.navflag=false;
    }
    else navbar.navflag=true;
  }
  },
}

</script>

<style >
  @import './common/font/font.css';
html,body
{
  width:100%;
  height:100%;
  overflow-x: hidden;
  margin: 0;
  padding: 0;
}
/* .footer_empty
{

  height:5%;
  width:100%;
} */
#app {
  font-family: 'Avenir', Helvetica, Arial, sans-serif;
  -webkit-font-smoothing: antialiased;
  -moz-osx-font-smoothing: grayscale;
  text-align: center;
  color: #2c3e50;
  width:100%;
  height:100%;
}
</style>
