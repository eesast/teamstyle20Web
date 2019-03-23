<template>
    <div id="error_contain">
    <h2>404 NOT FOUND</h2>
    <img class="eesast" src="../../static/img/eesast.png"/></img><h1>温馨提示</h1>
    <h3>访问页面错误!</h3>
    <h4>请检查网络连接是否正常，或网址是否正确</h4>
    <el-button type="primary" size="small" @click="gotoindex()">返回首页</el-button>
    
    </div>
</template>

<script>
export default {
    name: "error",
    data(){
        return {

        }
    },
    methods: {
        gotoindex()
        {
             this.$router.replace({
                path: '/empty_index',
             })
        }
    }
}
</script>


<style>
#error_contain
{
    position: relative;
    /* top:10%; */
    padding-top:9%;
    /* overflow: hidden; */
    height:80%;
}
#error_contain img
{
    /* display: inline; */
    height:30px;
    width:30px;
}
#error_contain img,#error_contain h1
{
    display:inline-flex;
    text-align: center;
    vertical-align: middle;
    color: #228DFF;
    padding: 4px;
    font-size:28px;
    position: relative;
    left:-5px;
}
#error_contain h2
{
    /* display: inline; */
    font-family: 'academy';
    color:aliceblue;
    font-size:88px;
    -webkit-animation: neon2 1.5s ease-in-out infinite alternate;
    -moz-animation: neon2 1.5s ease-in-out infinite alternate;
    animation: neon2 1.5s ease-in-out infinite alternate;
    position: relative;
    /* top:50px; */
}













@-webkit-keyframes neon2 {
  from {
    backgroud-position: 0 0;
    text-shadow: 0 0 10px #fff,
               0 0 20px  #fff,
               0 0 30px  #fff,
               0 0 40px  #228DFF,
               0 0 70px  #228DFF,
               0 0 80px  #228DFF,
               0 0 100px #228DFF,
               0 0 150px #228DFF;
  }
  to {
    background-position: 2000px 0;
    text-shadow: 0 0 5px #fff,
               0 0 10px #fff,
               0 0 15px #fff,
               0 0 20px #228DFF,
               0 0 35px #228DFF,
               0 0 40px #228DFF,
               0 0 50px #228DFF,
               0 0 75px #228DFF;
  }
}
@-moz-keyframes neon2 {
  from {
    backgroud-position: 0 0;
    text-shadow: 0 0 10px #fff,
               0 0 20px  #fff,
               0 0 30px  #fff,
               0 0 40px  #228DFF,
               0 0 70px  #228DFF,
               0 0 80px  #228DFF,
               0 0 100px #228DFF,
               0 0 150px #228DFF;
  }
  to {
   background-position: 2000px 0;
    text-shadow: 0 0 5px #fff,
               0 0 10px #fff,
               0 0 15px #fff,
               0 0 20px #228DFF,
               0 0 35px #228DFF,
               0 0 40px #228DFF,
               0 0 50px #228DFF,
               0 0 75px #228DFF;
  }
}
@keyframes neon2 {
  from {
    
    backgroud-position: 0 0;
    text-shadow: 0 0 10px #fff,
               0 0 20px  #fff,
               0 0 30px  #fff,
               0 0 40px  #228DFF,
               0 0 70px  #228DFF,
               0 0 80px  #228DFF,
               0 0 100px #228DFF,
               0 0 150px #228DFF;
  }
  to {
    background-position: 2000px 0;
    text-shadow: 0 0 5px #fff,
               0 0 10px #fff,
               0 0 15px #fff,
               0 0 20px #228DFF,
               0 0 35px #228DFF,
               0 0 40px #228DFF,
               0 0 50px #228DFF,
               0 0 75px #228DFF;
  }
} 
</style>