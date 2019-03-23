<template>
  
  <div class="footer">
    © 2019 <a href="https://future.eesast.com/">EESAST</a> All Rights Reserved.
    <!-- <div class="col supporter">
      <h4>赞助商</h4>
      <div class="supporter_container">
        <a href="">
          <img src="../../static/img/eesast.png"/><span style="color:#409EFF;">eesast</span>
          </a>
      </div>
    </div>
    
    <div class="col copyright">
      <h4 id="teamstyle" class="cpr">teamstyle20</h4>
     <h4 class="cpr">© 2019 <a href="https://future.eesast.com/">EESAST</a> All Rights Reserved.</h4>
    </div>
    
    <div class="col support">
      <h4>技术支持</h4>
      <a href="https://www.djangoproject.com/">
        <img src="../../static/img/django.svg" /></a>
      <a href="https://www.docker.com/">
        <img src="../../static/img/docker.svg" /></a> 
      <a href="https://cn.vuejs.org/">
        <img src="../../static/img/vue.png"/></a>
      <a href="https://element.eleme.io/">
        <img src="../../static/img/elementui.svg"/></a>
      <a href="https://github.com/">
        <img src="../../static/img/github.svg"/></a>
    </div> -->
  </div>
</template>

<script>
export default {
  name: "foot"
};

</script>

<style scoped>
/* @-webkit-keyframes neon2 {
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
} */
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
  margin-right:15%;
  font-family:'Balonb';
  font-size:36px;

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
html,body
{
  width:100%;
  overflow: hidden;
}
.footer {
  padding-top:15px; 
  width: 100%;
  /* height: 15%; */
  height:35px;
  /* min-height:120px; */
  /* display: flex;
  flex-direction: row; */
  /* background: rgb(109, 191, 230,0.2); */
  background: rgba(48, 48, 48, 0.3);
  color:white;
  position: relative;
  text-align: center;
  vertical-align: middle;
  /* margin-top:5%; */
}
.footer a
{
  text-decoration: none;
  color:white;
  font-weight: bolder;
}

.col {
  margin: 0;
  vertical-align: middle;
  box-sizing: content-box;
}

.supporter {
  order: 1;
  width: 10%;
  padding: 0 4%;
  vertical-align: middle;
  text-align: center;
  align-items: flex-start;
  /* p {
      margin: 10px;
    }
  a {
    font-size: 14px;
  }
  h4 {
    margin-bottom: 10px;
    padding-bottom: 10px;
    border-bottom: solid #f0f0f0 1px;
  } */
}
.supporter_container{
  margin-bottom: 12px;
}
.supporter_container a{
  font-size: 20px;
}
.supporter_container img{
  margin: 0;
}
.supporter a {
  text-decoration: none;
  color: black;
}
a:visited,a:hover {
  text-decoration: none;
}
a:hover{
  /* color: #303030da; */
}
.supporter p{
  margin-top: 0;
}
h4 {
  margin-bottom: 10px;
  padding-bottom: 10px;
  border-bottom: solid #f0f0f0 1px;
}

.supporter_container img {
  width: 20px;
  vertical-align: top;
  margin-right: 5%;
}


.copyright
{
  width:70%;
  order:3;
  padding: 0;
  text-align: center;
  vertical-align: middle;
}
.copyright .cpr
{
  display: inline;
  top:20px;
  position: relative;
  /* margin-right:100px; */
}
.copyright a
{
  text-decoration: none;
  font-weight: bolder;
  color: #303030da;
}



.support {
  order: 4;
  width: 20%;
  padding: 0 4%;
  text-align: center;
  align-items: flex-start;
}
.support h4{
  /* margin-bottom: 20px; */
  padding-bottom: 10px;
  border-bottom: solid #f0f0f0 1px;
}
.support img {
  max-width: 30px;
  height: auto;
  vertical-align: middle;
  /* margin: 20px 1%; */
}
 
 @media screen and (max-width: 720px)
 { 
   .footer
  {
    height:2em;
    top:9vh;
  }
   h4
  {
    font-size:1em;
  }
  .copyright .cpr
  {
    display:block;
    top:0em;
    
    font-size:0.8em;
    margin-right:0px;
    margin-top:0.8em;
    position: relative;
  }

  .supporter
  {
    width:24%;
  }
  /* .copyright
  {
    width:31%;
  } */
  .support
  {
    width:45%;
  }
    .support img {
    width: 1.5em;
    height: 1.5em;
    vertical-align: middle;
    margin: 0px 1%;
  }
   .supporter_container a {
  font-size:1em;
}

  
  #teamstyle
  {
    font-size:1.5em;
    top:0em;
    margin-right:0px;
  } 
 }
@media screen and (max-width: 768px)
{
  .copyright
  {
    display:none;
  }
  .supporter
  {
    width:30%;

  }
  .support
  {
    width:70%;
  }
} 
</style>
