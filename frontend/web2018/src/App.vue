<template>
  <div id="app">
    <navbar></navbar>
    <router-view/>
    <foot></foot>
  </div>
</template>

<script>
import navbar from './components/nav'
import foot from './components/foot'
export default {
  name: 'App',
  components:{navbar, foot}
}
</script>

<style >
  @import './common/font/font.css';
html,body
{
  width:100%;
  height:100%;
  overflow-x: hidden;
  margin: 0;
  padding: 0;
}
/* .footer_empty
{

  height:5%;
  width:100%;
} */
#app {
  font-family: 'Avenir', Helvetica, Arial, sans-serif;
  -webkit-font-smoothing: antialiased;
  -moz-osx-font-smoothing: grayscale;
  text-align: center;
  color: #2c3e50;
  width:100%;
  height:100%;
}
</style>
