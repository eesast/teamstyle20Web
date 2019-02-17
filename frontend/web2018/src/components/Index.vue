
<template>
  <div class="index" top="30px">
    <el-carousel height="300px" indicator-position="outside"  type="card">
      <el-carousel-item v-for="item in items" :key="item.id">
        <el-row>
          <el-col><img height="300"  ref="imgHeight" :src="item.idView" class="banner_img"/></el-col>
        </el-row>
      </el-carousel-item>
    </el-carousel>
    <div class="intro">
      <h4>这里是比赛简介</h4>
    </div>
    <el-button @click="indexjump" v-if="flag1">报名参加</el-button>
  </div>
</template>

<script>
import nav from '@/components/nav.vue'
export default{
  name:"index",
  data (){
    return{
      flag1 : true,
      items:[
        {id: 0, idView: require('..\\..\\static\\test\\1.jpg')},
        {id: 1, idView: require('..\\..\\static\\test\\2.jpg')},
        {id: 2, idView: require('..\\..\\static\\test\\4.jpg')}
      ]
    }
  },
  methods:{
    indexjump(){
        this.$router.push({path: '/login',query: {flag : true,ifnull : true}})
    }
  },
  computed:{
    show(){
      flag1 = nav.navflag;
    }
  }
}
</script>

<style>
.index
{
  min-height:70%;
  position: relative;
}
.el-carousel
{
  overflow: hidden;
}
.el-carousel__item{
}
  /*.el-carousel__item h3 {
    color: #475669;
    font-size: 14px;
    opacity: 0.75;
    line-height: 150px;
    margin: 0;
  }
  .el-carousel__item:nth-child(2n) {
     background-color: #99a9bf;
  }
  
  .el-carousel__item:nth-child(2n+1) {
     background-color: #d3dce6;
  }*/
</style>
