<template>
    <div id="logotitle_contain">
        <el-card shadow="always" style="background:#F8FBFF;">
        <img src="../../static/img/digit.png" class="cpr"></img>
        <h4 id="teamstyle" class="cpr">teamstyle20</h4>
        </el-card>
    </div>

</template>

<script>
export default {
    name: "logotitle",
    data(){
        return {

        }
    },
    methods:{

    }
}
</script>
<style>
#logotitle_contain
{
    width:100%;
    height:80px;
    margin-bottom:40px;
    position: relative;
}
#logotitle_contain img
{
    height:60px;
    width:auto;
    margin-right:30px;
}
#logotitle_contain .cpr
{
    display: inline;
    text-align: center;
    vertical-align: middle;
}
@keyframes ran {
    from {
        backgroud-position: 0 0;
    }
    to {
        background-position: 2000px 0;
    }
}
#teamstyle
{
  /* margin-right:15%; */
  font-family:'Balonb';
  /* color:#303030da; */
  font-size:50px;
  /* -webkit-animation: neon2 1.5s ease-in-out infinite alternate;
  -moz-animation: neon2 1.5s ease-in-out infinite alternate;
  animation: neon2 1.5s ease-in-out infinite alternate; */
  background: -webkit-linear-gradient(45deg, #F56C6C,#E6A23C,yellow,#67C23A,#409EFF,purple,#909399,#ffffff);
  -moz-linear-gradient:(45deg, #70f7fe, #fbd7c6, #fdefac, #bfb5dd, #bed5f5);
  -ms-linear-gradient:(45deg, #70f7fe, #fbd7c6, #fdefac, #bfb5dd, #bed5f5);
  color:transparent;
  animation: ran 30s linear infinite;
  /*设置字体颜色透明*/
  -webkit-background-clip:text;
  /*背景裁剪为文本形式*/
  /* animation: ran 10s linear infinite; */
}
@media screen and (max-width: 720px)
 { 
     #logotitle_contain
     {
         display: none;
     }
 }
</style>
