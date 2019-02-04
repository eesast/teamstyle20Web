<template>
  
  <div class="footer">
    <div class="supporter">
      <h4>赞助商</h4>
      <div class="supporter_container">
        <a href="">
          <img src="..\..\static\github.svg"/>Github</a>
      </div>
    </div>
    
    <div class="copyright">
      <h4>teamstyle20
      &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;  
      © 2019 <a href="https://future.eesast.com/">EESAST</a> All Rights Reserved.</h4>
    </div>
    
    <div class="support">
      <h4>技术支持</h4>
      <a href="https://www.djangoproject.com/">
        <img src="../../static/django.svg" /></a>
      <a href="https://www.docker.com/">
        <img src="../../static/docker.svg" /></a> 
      <a href="https://cn.vuejs.org/">
        <img src="../../static/vue.png"/></a>
      <a href="https://element.eleme.io/">
        <img src="../../static/elementui.svg"/></a>
      <a href="https://github.com/">
        <img src="../../static/github.svg"/></a>
    </div>
  </div>
</template>

<script>
export default {
  name: "foot"
};

</script>

<style scoped>
html,body
{
  width:100%;
  overflow: hidden;
}

.footer {
  width: 100%;
  height: 150px;
  display: flex;
  flex-direction: row;
  background: rgb(109, 191, 230,0.2);
  /* margin-top:20px; */
}

.col {
  margin: 0;
  vertical-align: middle;
  box-sizing: content-box;
}

.supporter {
  order: 1;
  width: 15%;
  padding: 0 4%;
  vertical-align: middle;
  text-align: center;
  align-items: flex-start;
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
a:visited a:hover {
  text-decoration: none;
}
a:hover{
  color: #303030da;
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
  order:3;
  padding: 0;
  text-align: center;
}
.copyright a
{
  text-decoration: none;
  font-weight: bolder;
  color: #303030da;
}



.support {
  order: 4;
  width: 34%;
  padding: 0;
  text-align: center;
  align-items: flex-start;
}
.support h4{
  margin-bottom: 20px;
  padding-bottom: 10px;
  border-bottom: solid #f0f0f0 1px;
}
.support img {
  width: 30px;
  height: 30px;
  vertical-align: middle;
  margin: 20px 1%;
}
</style>
