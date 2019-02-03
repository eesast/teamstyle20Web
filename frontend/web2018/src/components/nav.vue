<template>
  <div>

    <el-menu class="submenu" mode="horizontal">

      <el-menu-item index="0" id='img-submenu' route="/">
        <img class="eesast" src="..\..\static\eesast.png"/>
      </el-menu-item>
      <el-menu-item index="1" route="/">首页</el-menu-item>
      <el-menu-item v-if="navflag" index="2" route="/login">登录/注册</el-menu-item>
      <el-menu-item v-if!="navflag" index="3" route="">组队</el-menu-item>
      <el-menu-item v-if!="navflag" index="4" route="">对战</el-menu-item>
      <el-menu-item v-if!="navflag" index="5" route="">个人中心</el-menu-item>
      <el-menu-item index="6" route="">文件</el-menu-item>
      <el-menu-item index="7" route="">公告</el-menu-item>
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
