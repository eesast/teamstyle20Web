<template>
  <div id="nav">
    <i class="el-icon-menu" id="menuicon" @click="Showmenu()" style="font-size: 6vh;"><span style="font-size:4.5vh;left:5vw;position:relative;">Teamstyle20</span></i>
    
    <el-menu v-show="showmenu"class="submenu" :mode="whatmode" v-bind:router="true" :default-active="onRoutes" @select="handleSelect" router>

      <el-menu-item index="#" id='img-submenu' >
        <img class="eesast" src="../../static/img/eesast.png"/>
      </el-menu-item>
      <el-menu-item index="/empty_index" >首页</el-menu-item>
      <el-menu-item v-show="navflag==true" index="/login" >登录/注册</el-menu-item>
      <el-menu-item v-show="navflag!=true" index="/team" >组队</el-menu-item>
      <el-menu-item v-show="navflag!=true" index="/battle" >对战</el-menu-item>
      <el-menu-item v-show="navflag!=true" index="/personal" >个人中心</el-menu-item>
      <el-menu-item index="/file" >文件</el-menu-item>
      <el-menu-item index="/announcement">公告</el-menu-item>
      <el-menu-item index="/QR">二维码</el-menu-item>
      <el-menu-item v-show="navflag!=true" class="bu1" index="" style="float:right;"> <div  @click="navjump" style="vertical-align:middle;color:#F56C6C;">退出登录</div></el-menu-item>
      <el-menu-item index="/personal" v-show="navflag!=true" class="bu1" style="float:right;"> {{name}} </el-menu-item>
    </el-menu>
  </div>
</template>

<script>

import { EventBus } from "../bus.js"
export default {
    name: "navbar",
    data() {
      return {
        navflag: true,
      //  navflag: false,
       name : "test",
       showmenu:true,
       whatmode:"horizontal",
      }
    },
     watch:{
  $route(to,from){
    console.log(to.path);
    // if(to.path=="/")
    // {
    //   // 跳转到首页
    //   window.location="http://teamstyle.eesast.com";
    // }
    if(getCookie('token')!=null)
    {
      this.navflag=false;
    }
    else this.navflag=true;
  }
  },
  methods: {
        handleSelect(key, keyPath) {
            console.log(key, keyPath);
        },
        navjump(){
          this.navflag = true;
          delCookie(token)
          delCookie(id)
          delCookie(username)
          this.$router.push({path: '/empty_index'})
        },
        Showmenu()
        {
          this.showmenu=!this.showmenu;
        }
    },

  computed:{
    onRoutes(){
    //if(this.$route.query.ifnull){
        //this.navflag = this.$route.query.flag;
     //}
   // else {};
   return this.$route.path;
    }
  },
  mounted(){
            EventBus.$on("send-msg", ({flag}) => {
                this.navflag =flag;}
            )
            var width=window.screen.width;
            // console.log(width);
            if(width<720)
            {
              this.showmenu=false;
              this.whatmode="vertical";     
              // plus.screen.lockOrientation("portrait-primary"); 
              
            }
  }
}

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

</script>

<style scoped>
#menuicon
{
  display: none;
}
#nav
{
  height:60px;
}
#nav img{
  height:40px;
  width:40px;
}

.el-menu{
    top:0px;
    height:100%;
}
.bu1{
  float: right;
}
.submenu
{
  z-index:2000;
}
  @media screen and (max-width:720px) {
     #nav .el-menu {
        position: fixed;
        /* top: 8vh; */
        height: 100vh;
        width: 30vw!important;
        z-index: 2000;
        background-color: #EBEEF5;
        border: none;
        /* border-right: solid 1px #fefefe; */
    }
    #nav .el-menu-item {
      font-size:2vh;
      top:8vh;
      height:8vh;
      width:100%;
    }
     #nav .el-menu-item.is-active {
      background: #FFFFFF;
    }
    #nav
    {
       display: block;
        width: 100%;
        height: 8vh;
        position: fixed;
        top: 0;
        z-index: 2010;
        background-color: #EBEEF5;
        border-bottom: solid 1px #fefefe;
    }
    #menuicon
    {
      display: block;
      position: absolute;
      height: 8vh;
      top:1vh;
      left:6vw;
      z-index: 2011;
    }
    /* #nav {
        display: block;
        width: 100%;
        height: 100vh;
        position: fixed;
        top: 0;
        z-index: 2010;
        background-color: rgba(250, 250, 250, 0.9);
    } */
    /* .el-menu-item-group{
        background-color: #f4f4f4;
        width: 200px;
    } */
  }
</style>
