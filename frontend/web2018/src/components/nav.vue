<template>
  <div id="nav">

    <el-menu class="submenu" mode="horizontal" v-bind:router="true">

      <el-menu-item index="0" id='img-submenu' >
        <img class="eesast" src="..\..\static\img\eesast.png"/>
      </el-menu-item>
      <el-menu-item index="/" >首页</el-menu-item>
      <el-menu-item v-if="navflag==true" index="/login" >登录/注册</el-menu-item>
      <el-menu-item v-if="navflag!=true" index="/team" >组队</el-menu-item>
      <el-menu-item v-if="navflag!=true" index="/battle" >对战</el-menu-item>
      <el-menu-item v-if="navflag!=true" index="/personal" >个人中心</el-menu-item>
      <el-menu-item index="/file" >文件</el-menu-item>
      <el-menu-item index="/announcement">公告</el-menu-item>
    </el-menu>
  </div>
</template>

<script>
export default {
    name: "navbar",
    data() {
      return {
        // navflag: true
        navflag:false
      }
    },
  methods: {
        handleSelect(key, keyPath) {
            console.log(key, keyPath);
        }
    }
};
</script>

<style scoped>
#nav
{
  height:10%;
}
#nav img{
  height:60px;
  width:60px;
}

el-menu{
    top:0px;
    height:100px;
}
</style>
