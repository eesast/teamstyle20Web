<template>
  <div id="login">
    <el-tabs v-model="activeName" type="border-card" @tab-click="handleClick">
      <el-tab-pane label="登录" name="login">
      <el-form :model="form" :rules="rules" ref="form" label-width="100px">
        <el-form-item prop="name" label="账号">
         <el-input v-model="form.name"></el-input>
       </el-form-item>
       <el-form-item prop="password" label="密码">
         <el-input v-model="form.password"></el-input>
        </el-form-item>
        <el-form-item>
         <el-button type="primary" @click="submit">登录</el-button> 
         </el-form-item>
    </el-form>
    </el-tab-pane>
      <el-tab-pane label="注册" name="register">
      <el-form :model="form" :rules="rules" ref="form" label-width="100px">
        <el-form-item prop="name" label="账号">
         <el-input v-model="form.name"></el-input>
       </el-form-item>
       <el-form-item prop="password" label="密码">
         <el-input v-model="form.password"></el-input>
       </el-form-item>
       <el-form-item prop="password" label="确认密码">
         <el-input v-model="form.password1"></el-input>
       </el-form-item>
        <el-form-item prop="realname" label="真实姓名">
         <el-input v-model="form.realname"></el-input>
       </el-form-item>
       <el-form-item prop="class" label="班级">
        <el-input v-model="form.class"></el-input>
      </el-form-item>
      <el-form-item prop="studentid" label="学号">
        <el-input v-model="form.studentid"></el-input>
      </el-form-item>
      <el-form-item prop="phone" label="手机号">
        <el-input v-model="form.phone"></el-input>
      </el-form-item>
      <el-form-item prop="email" label="邮箱">
        <el-input v-model="form.email"></el-input>
      </el-form-item>
      <el-form-item>
      <el-button type="primary" @click="submit">提交</el-button> 
      </el-form-item>
    </el-form>
    </el-tab-pane>
    <el-tab-pane label="帮助" name="help">
      <el-form :model="form" :rules="rules" ref="form" label-width="100px">
        <el-form-item prop="name" label="账号">
         <el-input v-model="form.name"></el-input>
        </el-form-item>
        <el-form-item prop="realname" label="真实姓名">
         <el-input v-model="form.realname"></el-input>
       </el-form-item>
       <el-form-item prop="class" label="班级">
        <el-input v-model="form.class"></el-input>
      </el-form-item>
      <el-form-item prop="studentid" label="学号">
        <el-input v-model="form.studentid"></el-input>
      </el-form-item>
      <el-form-item prop="phone" label="手机号">
        <el-input v-model="form.phone"></el-input>
      </el-form-item>
      <el-form-item prop="email" label="邮箱">
        <el-input v-model="form.email"></el-input>
      </el-form-item>
      <el-form-item>
      <el-button type="primary" @click="submit">找回账号</el-button> 
      </el-form-item>
    </el-form>
    </el-tab-pane>
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
          realname: [
            {required: true, message: '请输入姓名', trigger: 'blur'}
          ],
          class: [
            {required: true, message: '请输入班级', trigger: 'blur'}
          ],
          sex: [
            {required: true, message: '请选择', trigger: 'blur'}
          ],
          email: [
            {required: true, message: '请输入邮箱', trigger: 'blur'}
          ],
          name:[
            {required: true, message: '请输入账号名称', trigger: 'blur'}
          ],
          password:[
            {required: true, message: '请输入密码', trigger: 'blur'}
          ],
          phone:[
            {required: true, message: '请输入手机号', trigger: 'blur'}
          ],
          studentid:[
            {required: true, message: '请输入学号', trigger: 'blur'}
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
</style>
