<template>
  <div id="app">
    <!-- <img src="./assets/logo.png"> -->
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

<style>
#app {
  font-family: 'Avenir', Helvetica, Arial, sans-serif;
  -webkit-font-smoothing: antialiased;
  -moz-osx-font-smoothing: grayscale;
  text-align: center;
  color: #2c3e50;
  margin-top: 60px;
}
</style>
