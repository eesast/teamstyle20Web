<template>
  <div class="index" top="30px">
    <el-carousel height="300px" indicator-position="outside"  type="card">
      <el-carousel-item v-for="item in items" :key="item">
        <h3>{{ item }}</h3>
      </el-carousel-item>
    </el-carousel>
    <div class="intro" height="300px">
      <h4>这里是比赛简介</h4>
    </div>
    <el-button>报名参加</el-button>
  </div>
</template>

<script>
export default{
  name:"index",
  data (){
    return{
      items:[
        1,2,3,4,5
      ]
    }
  }
}
</script>

<style>
.el-carousel__item{
  margin-top:30px;
}
  .el-carousel__item h3 {
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
  }
</style>
