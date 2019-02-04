<template>
  <div class="hello">
    <router-link to="announcement" class="link-to-announcement" @click.native="" >announcement</router-link>
  </div>
</template>

<script>
export default {
  name: 'HelloWorld',
  data () {
    return {
      msg: 'Welcome to Your Vue.js App'
    }
  },
  methods:
  {
    selectnav(selectindex)
    {
      console.log(selectindex);
      // let selectedindex=selectindex;
      // localStorage.setItem('id',selectedindex);
    }
  }
}
</script>

<!-- Add "scoped" attribute to limit CSS to this component only -->
<style scoped>
html,body
{
  width:100%;
}
</style>
