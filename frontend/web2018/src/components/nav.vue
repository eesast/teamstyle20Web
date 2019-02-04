<template>
  <div>

    <el-menu class="submenu" mode="horizontal" router=true>

      <el-menu-item index="0" id='img-submenu' router="/">
        <img class="eesast" src="..\..\static\eesast.png"/>
      </el-menu-item>
      <el-menu-item index="1" router="/">首页</el-menu-item>
      <el-menu-item v-if="navflag==true" index="/login" >登录/注册</el-menu-item>
      <el-menu-item v-if="navflag!=true" index="3" router="">组队</el-menu-item>
      <el-menu-item v-if="navflag!=true" index="4" router="">对战</el-menu-item>
      <el-menu-item v-if="navflag!=true" index="5" router="">个人中心</el-menu-item>
      <el-menu-item index="6" router="">文件</el-menu-item>
      <el-menu-item index="/announcement">公告</el-menu-item>
    </el-menu>
  </div>
</template>

<script>
export default {
    name: "navbar",
    data() {
      return {
        navflag: true
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
img{
  height:60px;
  width:60px;
}

el-menu{
    top:0px;
    height:100px;
}
</style>
