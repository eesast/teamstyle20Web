<template>
  <div id="nav">
    <el-menu class="submenu" mode="horizontal" v-bind:router="true" :default-active="onRoutes" @select="handleSelect" router>

      <el-menu-item index="0" id='img-submenu' >
        <img class="eesast" src="..\..\static\img\eesast.png"/>
      </el-menu-item>
      <el-menu-item index="/empty_index" >首页</el-menu-item>
      <el-menu-item v-show="navflag==true" index="/login" >登录/注册</el-menu-item>
      <el-menu-item v-show="navflag!=true" index="/team" >组队</el-menu-item>
      <el-menu-item v-show="navflag!=true" index="/battle" >对战</el-menu-item>
      <el-menu-item v-show="navflag!=true" index="/personal" >个人中心</el-menu-item>
      <el-menu-item index="/file" >文件</el-menu-item>
      <el-menu-item index="/announcement">公告</el-menu-item>
      <el-menu-item index="/QR">二维码</el-menu-item>
      <el-menu-item v-show="navflag!=true" class="bu1" index=""> <div  @click="navjump" style="vertical-align:middle;color:#F56C6C;">退出登录</div></el-menu-item>
      <el-menu-item index="/personal" v-show="navflag!=true" class="bu1"> {{name}} </el-menu-item>
    </el-menu>
  </div>
</template>

<script>
import { EventBus } from "../bus.js"
export default {
    name: "navbar",
    data() {
      return {
        // navflag: true
       navflag: false,
       name : "test"
      }
    },
  methods: {
        handleSelect(key, keyPath) {
            console.log(key, keyPath);
        },
        navjump(){
          this.navflag = true;
          this.$router.push({path: '/empty_index'})
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
  }
}
</script>

<style scoped>
#nav
{
  height:10%;
}
#nav img{
  height:40px;
  width:40px;
}

el-menu{
    top:0px;
    height:100px;
}
.bu1{
  float: right
}


  @media screen and (max-width:720px) {
    /* #nav el-menu-item
    {
      font-size:10px;
    } */
  }
</style>
