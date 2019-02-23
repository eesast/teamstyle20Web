<template>
  <div id="login">
   <el-tabs v-model="activeName" type="border-card" @tab-click="handleClick">
      <el-form :model="form" :rules="rules" ref="form" label-width="100px">
        <el-button id="al-bu1">获取验证码</el-button>
        <el-form-item prop="code" label="验证码">
         <el-input v-model="form.name"></el-input>
       </el-form-item>
       <el-form-item prop="password" label="密码">
         <el-input v-model="form.password"></el-input>
        </el-form-item>
        <el-form-item prop="password" label="确认密码">
         <el-input v-model="form.password"></el-input>
        </el-form-item>
        <el-form-item>
         <el-button type="primary" @click="submit">提交</el-button> 
         </el-form-item>
    </el-form>
    </el-tabs>
  </div>
</template>

<script>
export default {
  name: 'Register',
  data(){
      return {
        activeName:"login",
        form: {
          realname: ''
        },
        rules: {
          password:[
            {required: true, message: '请输入密码', trigger: 'blur'}
          ],
          code:[
            {required: true, message: '请输入验证码', trigger: 'blur'}
          ]
        }
      }
  },
  methods: {
    submit(){
        
    },
    handleClick(tab, event) {
        console.log(tab, event);
      }
  }
}
</script>

<style scoped>
#login
{
  min-height: 70%;
  margin-bottom:50px;
}
  .el-form-item{
    width:40%;
    position: relative;
    left:10%;
  }
  .el-input{
    display: inline-block;
  }
  .el-tabs{
    width:80%;
    position: relative;
    left:10%;
    top:30px;
  }
  #al-bu1{
    float: right
  }
</style>
