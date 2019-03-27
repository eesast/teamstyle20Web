<template>
  <div id="app">
    <navbar v-if="$route.name.name !== 'error'"></navbar>
    <!-- <logotitle v-if="$route.name.name !== 'error' && $route.name.name !== 'index'"></logotitle> -->
    <router-view/>
    <foot v-if="$route.name.name !== 'error'&& $route.name.name !== 'index'"></foot>
  </div>
</template>

<script>
function getCookie(cname){
    var name = cname + "=";
    var ca = document.cookie.split(';');
    for(var i=0; i<ca.length; i++) {
        var c = ca[i].trim();
        if (c.indexOf(name)==0) { return c.substring(name.length,c.length); }
    }
    return "";
}

function delCookie(name)
{
var exp = new Date();
exp.setTime(exp.getTime() - 1);
var cval=getCookie(name);
if(cval!=null)
document.cookie= name + "="+cval+";expires="+exp.toGMTString()+";path=/";
}


import navbar from './components/nav'
import logotitle from './components/logotitle'
import foot from './components/foot'

console.log(getCookie('token'));
if(getCookie('token')!='null'&&getCookie('token')!=null&&getCookie('token')!='')
{
  navbar.navflag=false;
}
else navbar.navflag=true;

export default {
  name: 'App',
  components:{navbar, foot,logotitle},
   watch:{
  $route(to,from){
    console.log(to.path);
    if(to.path=="/")
    {
      // 跳转到首页
      window.location="http://teamstyle.eesast.com";
       
    }
    console.log(getCookie('token'));
   if(getCookie('token')!='null'&&getCookie('token')!=null&&getCookie('token')!='')
    {
      navbar.navflag=false;
    }
    else navbar.navflag=true;
  }
  },
  mounted:function()
  {
    console.log(this.$route.name);
    console.log(getCookie('token'));
    if(getCookie('token')!='null'&&getCookie('token')!=null&&getCookie('token')!='')
    {
      navbar.navflag=false;
    }
    else navbar.navflag=true;
  }
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
@media screen and (max-width:720px) {
.el-message-box
{
  position: fixed;
  
  width:90vw!important;;
  /* display: block; */
  top:40vh;
  left:5vw;
}
}
</style>
