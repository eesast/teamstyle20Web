<template>
  <div id="login">
    <el-alert
    center
    style="width:80%;left:10%;color:#409EFF;background-color:#ecf5ff;"
    title="提示:本界面注册的账号还可以用于登录电子系科协的其他网站"
    type="info"
    show-icon>
    </el-alert>
    <el-tabs v-model="activeName" type="border-card" @tab-click="handleClick">
      <el-tab-pane label="登录" name="login" align="center">
      <el-form :model="form" :rules="rules" ref="form" label-width="60px" >
        <el-form-item prop="name" label="账号">
         <el-input v-model="form.name"></el-input>
       </el-form-item>
       <el-form-item  prop="password" label="密码">
         <el-input type="password" v-model="form.password" show-password></el-input>
        </el-form-item>
        <el-form-item>
         <el-button type="primary" @click="log_in" style="position:relative;margin-left:-60px;">登录</el-button> 
         </el-form-item>
    </el-form>
    </el-tab-pane>
      <el-tab-pane label="注册" name="register" align="center">
      <el-form :model="form" :rules="rules" ref="form" label-width="80px">
        <el-form-item prop="name" label="账号">
         <el-input v-model="form.name"></el-input>
       </el-form-item>
       <el-form-item prop="password" label="密码">
         <el-input v-model="form.password" type="password"></el-input>
       </el-form-item>
       <el-form-item prop="password" label="确认密码">
         <el-input v-model="form.password1" type="password"></el-input>
       </el-form-item>
        <el-form-item prop="realname" label="真实姓名">
         <el-input v-model="form.realname"></el-input>
       </el-form-item>
       <el-form-item prop="classx" label="班级">
        <el-input v-model="form.classx"></el-input>
      </el-form-item>
      <el-form-item prop="studentid" label="学号">
        <el-input v-model="form.studentid"></el-input>
      </el-form-item>
      <el-form-item prop="phone" label="手机号">
        <el-input v-model="form.phone"></el-input>
      </el-form-item>
      <el-form-item prop="department" label="院系">
        <el-input v-model="form.department"></el-input>
      </el-form-item>
      <el-form-item prop="email" label="邮箱">
        <el-input v-model="form.email"></el-input>
      </el-form-item>
      <el-form-item>
      <el-button type="primary" @click="create" style="position:relative;margin-left:-60px;">提交</el-button> 
      </el-form-item>
    </el-form>
    </el-tab-pane>
    <el-tab-pane label="帮助" name="help" align="center">
      <el-form :model="form" :rules="rules" ref="form" label-width="80px">
        <el-form-item prop="name" label="账号">
         <el-input v-model="form.name"></el-input>
        </el-form-item>
        <el-form-item prop="realname" label="真实姓名">
         <el-input v-model="form.realname"></el-input>
       </el-form-item>
       <el-form-item prop="classx" label="班级">
        <el-input v-model="form.classx"></el-input>
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
      <el-button type="primary" @click="find" style="position:relative;margin-left:-60px;">找回账号</el-button> 
      </el-form-item>
    </el-form>
    </el-tab-pane>
    </el-tabs>
  </div>
</template>

<script>
import nav from '@/components/nav.vue'
export default {
  name: 'Register',
  props: ['realname','name','password','classx','realname','studentid','phone','email','department'],
  data(){
      return {
        activeName:"login",
        form:{
          realname:"",
          name:"",
          password:"",
          password1:"",
          classx:"",
          studentid:"",
          phone:"",
          email:"",
          department:"",
          token:"",
          id:""
        },
        rules: {
          realname: [
            {required: true, message: '请输入姓名', trigger: 'blur'}
          ],
          classx: [
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
          ],
          department:[
            {required: true, message: '请输入邮箱', trigger: 'blur'}
          ]
        }
      }
  },

  created:function()
  {
    // setCookie("usename",null)
    // setCookie("id",null)
    // setCookie("token",null)
    //不重新设置，过期就过期
    // this.$router.push({path: '/',query: {flag : false}})
  },

  methods: {
    log_in:function(){
      fetch("/api/auth",
      {
        method:'POST',
        headers:
        {
            'Content-Type':'application/json',
           // 'Access-Control-Allow-Origin':'https://teamstyle.eesast.com',
            "Access-Control-Allow-Credentials":"true",
            "Access-Control-Allow-Headers":"Access-Control-Allow-Headers, Origin,Accept, X-Requested-With, Content-Type, Access-Control-Request-Method, Access-Control-Request-Headers",
            "Access-Control-Allow-Methods":"GET,HEAD,OPTIONS,POST,PUT"
        },
        body:JSON.stringify(
            {
                'username':this.form.name,
                'password':this.form.password
            }
        )
      }).then(response=>
      {
        // console.log(response)
        if(response.ok)
        {
          return response.json();
        }
        else if(response.status=='404')
        {
          this.$message.error("您输入的用户不存在！");
        }
        else if(response.status=='422')
        {
          this.$message.error("用户名或密码字段不应为空！");
        }
        else if(response.status=='401')
        {
          this.$message.error("用户名或密码错误！");
        }
        else
        {
          this.$message.error("登录失败！");
        }
      }).then(res=>
      {
        if (res==undefined) return
        this.form.token=res['token']
        this.form.id=res['id']
        this.form.username=res['username']
        this.$message.success('登录成功！');
        nav.navflag=false
        setCookie("token",this.form.token)
        setCookie("id",this.form.id)
        setCookie("username",this.form.username)
        setTimeout(() => {
          this.$router.push({path: '/',query: {flag : false}})
        }, 100);
      })
    },
    create:function(){
      // console.log(this.form.name)
      // console.log(this.form.password);
      // console.log(this.form.password1);
      if(this.form.password!=this.form.password1)
      {
        this.$message.error('两次输入的密码不一样!');
        return ;
      }
      fetch("/api/users",
      {
        method:'POST',
        headers:
        {
            'Content-Type':'application/json',
            //'Access-Control-Allow-Origin':'https://teamstyle.eesast.com',
            //"Access-Control-Allow-Credentials":"true",
            //"Access-Control-Allow-Headers":"Access-Control-Allow-Headers, Origin,Accept, X-Requested-With, Content-Type, Access-Control-Request-Method, Access-Control-Request-Headers",
            "Access-Control-Allow-Methods":"GET,HEAD,OPTIONS,POST,PUT"
        },
        body:JSON.stringify(
            {
                "username":this.form.name,
                "password":this.form.password,
                "phone":this.form.phone,
                "email":this.form.email,
                "name":this.form.realname,
                "classx":this.form.class,
                "id":this.form.studentid,
                "department":this.form.department
            }
        )
      }).then(response=>
      {
        // console.log(response.status)
        // console.log(response)
        if(response.status=='201')
        {
          return response.json();
        }
        else if(response.status=='409')
        {
          this.$message.error("用户名、邮箱或学号冲突冲突，请更改后输入！");
        }
        else if(response.status=='422')
        {
          this.$message.error("您尚未填写完整信息！");
        }
        else
        {
          this.$message.error("注册失败！");
        }
      }).then(res=>
      {
        // console.log(res)
        this.form.token=res['token']
        this.$message.success('注册成功，请登录开始您的挑战！');
        nav.navflag=false
        setCookie("id",this.form.id)
        setCookie("username",this.form.username)
        setCookie("token",token)
        setTimeout(() => {
          this.$router.push({path: '/',query: {flag : false} })
        }, 100);
      })
    },
    find:function(){
      //it will be put into use until backend is finished
    },
    submit(){
        this.$router.push({path: '/',query: {flag : false,ifnull : true}})
    },
    handleClick(tab, event) {
        // console.log(tab, event);
        //init所有formdata
        // form:{
          this.form.realname=""
          this.form.name=""
          this.form.password=""
          this.form.password1=""
          this.form.classx=""
          this.form.studentid=""
          this.form.phone=""
          this.form.email=""
          this.form.department=""
          
        // },
      },
  }
}

function setCookie(cname,cvalue){
      document.cookie = cname+"="+cvalue+";path=/";
    }

</script>

<style scoped>
#login
{
  min-height: 70%;
  margin-bottom:50px;
}

#login div.el-alert__content .el-alert__description
{
  color:red;
}
#login  .el-form-item{
    width:40%;
    position: relative;
    /* left:10%; */
  }
#login  .el-input{
    display: inline-block;
  }
 #login .el-tabs{
    width:80%;
    position: relative;
    left:10%;
    top:30px;
  }
@media screen and (max-width: 768px)
{
  #login{
    top:9vh;
    position: relative;
    min-height:77%;
    margin-bottom:18vh;
    /* width:100%; */
  }
  #login .el-tabs{
    width:90%;
    position: relative;
    left:5%;
    top:30px;
  }
  #login  .el-form-item{
    width:100%;
    position: relative;
    /* left:10%; */
  }
}  
</style>
