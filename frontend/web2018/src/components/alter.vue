<template>
  <div id="login">
   <el-tabs v-model="activeName" type="border-card" @tab-click="handleClick" align="center">
      <el-form :model="form" :rules="rules" ref="form" label-width="100px">
        
        <!-- <el-form-item prop="code" label="验证码">
         <el-input v-model="form.name"></el-input><el-button id="al-bu1">获取验证码</el-button>
       </el-form-item> -->
       <el-form-item prop="password" label="原密码">
         <el-input type="password" v-model="form.oldpassword"></el-input>
        </el-form-item>
       <el-form-item prop="password" label="新密码">
         <el-input type="password" v-model="form.password"></el-input>
        </el-form-item>
        <el-form-item prop="passwordagain" label="确认密码">
         <el-input type="password" v-model="form.password1"></el-input>
        </el-form-item>
        <el-form-item>
         <el-button type="primary" @click="submit" style="margin-left:-60px;position:relative;">提交</el-button> 
         </el-form-item>
    </el-form>
    </el-tabs>
  </div>
</template>

<script>

function getCookie(cname) {
  var name = cname + "=";
  var ca = document.cookie.split(";");
  for (var i = 0; i < ca.length; i++) {
    var c = ca[i].trim();
    if (c.indexOf(name) == 0) {
      return c.substring(name.length, c.length);
    }
  }
  return "";
}
function delCookie(name)
{
var exp = new Date();
exp.setTime(exp.getTime() - 1);
var cval=getCookie(name);
if(cval!=null)
document.cookie= name + "="+cval+";expires="+exp.toGMTString()+";path=/";
}
var token = getCookie("token");
if(token==null)
{
    this.$message("您尚未登录！")
    setTimeout(() => {
          this.$router.push({path: '/index'})
        }, 100);
}
var username = getCookie("username");
var id = parseInt(getCookie("id"));

export default {
  name: 'Register',
  data(){
      return {
        activeName:"login",
        form: {
          oldpassword:'',
          password:'',
          password1:'',
        },
        rules: {
          password:[
            {required: true, message: '请输入密码', trigger: 'blur'}
          ],
          passwordagain:[
            {required: true, message: '请再次输入密码', trigger: 'blur'}
          ],
          code:[
            {required: true, message: '请输入验证码', trigger: 'blur'}
          ]
        }
      }
  },
  methods: {
    submit(){
        if(this.form.password!=this.form.password1)
        {
          this.$message.error('两次输入的密码不一样!')
          return ;
        }
        else
        {
          //尝试检测是否是正确的密码，然后发送修改
          fetch("/api/auth",
          {
            method:'POST',
            headers:
            {
                'Content-Type':'application/json'
            },
            body:JSON.stringify(
                {
                    'username':username,
                    'password':this.form.oldpassword
                }
            )
          }).then(response=>{
            if(response.ok)
            {
              //验证成功，修改密码
              fetch('/api/users/'+id,{
                method:'PUT',
                headers:
                {
                  'Content-Type':'application/json',
                  "x-access-token":JSON.stringify({"token":token,"id":id,"username":username,"auth":true})
                },
                body:JSON.stringify(
                {
                    'password':this.form.password,
                }
                )
              }).then(response=>{
                if(response.status=="204")
                {
                  this.$message.success('密码修改成功,请重新登录!');
                  delCookie('token')
                  delCookie('id')
                  delCookie('username')
                   setTimeout(() => {
                    this.$router.push({path: '/'})
                  }, 400);
                }
                else if(response.status=="401")
                {
                  this.$message.error('登录已失效，请重新登录!');
                  if(token!=null)
                  {
                      delCookie("token")
                      delCookie("id")
                      delCookie("username")
                      token=null
                      username=null
                      id=null
                      setTimeout(() => {
                        window.location="https://teamstyle.eesast.com/login";
                      }, 100)
                  }
                  // delCookie('token')
                  // delCookie('id')
                  // delCookie('username')
                  // setTimeout(() => {
                  //   this.$router.push({path: '/'})
                  // }, 400);
                }
                else if(response.status=="404")
                {
                  this.$message.error('用户不存在!');
                }
                else
                {
                  this.$message.error('修改失败!')
                }
              })


              // return response.json();
            }
            else
            {
              this.$message.error('原密码错误!');
              return ;
            }
          })



        }
    },
    handleClick(tab, event) {
        //console.log(tab, event);
      }
  }
}
</script>

<style scoped>
#login
{
  min-height: 80%;
  margin-bottom:50px;
}
 #login .el-form-item{
    width:40%;
    position: relative;
    /* left:10%; */
  }
 #login .el-input{
    display: inline-block;
  }
 #login .el-tabs{
    width:80%;
    position: relative;
    left:10%;
    top:30px;
  }
  #al-bu1{
  float: left;
  margin-top:2vh;
}
  @media screen and (max-width:720px) {
      #login
      {
        min-height: 80vh;
        position: relative;
        top:10vh;
        margin-bottom:50px;
      }
       #login .el-form-item{
        width:100%;
        position: relative;
        left:0%;
      }
       
  }
</style>
