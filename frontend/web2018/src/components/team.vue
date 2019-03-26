<template>
    <div id="team_contain">
        <el-tabs type="border-card" id="team_card">
        <el-tab-pane label="我的队伍">

            <!-- 没有加入队伍时 -->
            <div v-if="inteam==false" id="create_team">
            <el-alert
            class="create_team"
            title="您还没有队伍哦"
            :closable="false"
            type="info"
            center
            show-icon>
            </el-alert>
            <el-button type="primary" class="create_team" @click="createDialog">创建队伍</el-button>
            <el-dialog id="create_team_dialog" title="创建队伍" :visible.sync="dialogFormVisible" center>
                <el-form :model="form" :rules="rules"> 
                    <!-- status-icon> -->
                    <el-form-item prop="teamname" label="队伍名称" :label-width="formLabelWidth">
                    <el-input v-model="form.teamname" autocomplete="off" placeholder="队伍名称不超过10个字"></el-input>
                    </el-form-item>
                    <el-form-item prop="description" label="队伍简介" :label-width="formLabelWidth">
                    <el-input type="textarea" :rows="8" resize="none"  v-model="form.description" autocomplete="off" placeholder="队伍简介不超过50个字"></el-input>
                    </el-form-item>
                </el-form>
                <div slot="footer" class="dialog-footer">
                    <el-button @click="dialogFormVisible = false">取 消</el-button>
                    <el-button type="primary" @click="createTeam()">创建队伍</el-button>
                </div>
            </el-dialog>
            </div>

            <!-- 加入队伍时 -->
            <div v-if="inteam==true" id="in_team">
                 <table  id="detail_table">
                    <tr class='table_th1'>
                        <th ><img src="../../static/img/star.svg" class="svg"></img>队伍名称</th>
                        <td>{{detailData.teamname}}</td><td>
                            <el-button v-if="iscaptain==true"type="danger" size="small" style="position:relative;float:right;" @click="clearTeam()">解散队伍</el-button>
                            <el-button v-else type="danger" size="small" style="position:relative;float:right;" @click="exitTeam()">退出队伍</el-button>
                        </td>
                    </tr>
                    <tr class='table_th2'>
                        <th ><img src="../../static/img/user.svg" class="svg"></img>队长</th>
                        <td>{{detailData.captain}}</td>
                        <td></td>
                    </tr>
                    <tr class='table_th1'>
                        <th ><img src="../../static/img/id card.svg" class="svg"></img>邀请码</th>
                        <td>{{detailData.invitecode}}</td><td><el-button v-clipboard:copy="detailData.invitecode" v-clipboard:success="onCopy" v-clipboard:error="onError" id="copyinvitecode"type="primary" size="small">复制</el-button></td>
                    </tr>
                    <tr class='table_th2'>
                        <th ><img src="../../static/img/edit-square.svg" class="svg"></img>队伍简介</th>
                        <td colspan="1">
                          <el-input type="textarea" :rows="6" resize="none"  v-model="detailData.description" autocomplete="off" :readonly="isediting"></el-input>
                        </td>
                        <td v-if="iscaptain">
                          <el-button v-if="isediting==true" size="small" type="primary" icon="el-icon-edit" style="float:right;top:5px;position:relative;" @click="edit_description()">修改简介</el-button>
                          <el-button v-else size="small" type="primary" icon="el-icon-edit" style="float:right;top:5px;position:relative;" @click="edit_description()">保存修改</el-button>
                        </td>
                    </tr>      
                    <tr class='table_th1'>
                        <th ><img src="../../static/img/team.svg"  class="svg"></img>队伍成员</th>
                        <td colspan="2">
                            <table id="members_table">
                                <template v-for="(x,index) in detailData.members">
                                <tr v-if="detailData.membersID[index]!=detailData.captainID" :class="'table_th'+(index%2+1)">
                                    <td  align="middle">{{detailData.members[index]}}
                                        <el-button v-if="iscaptain==true" size="small" type="danger" class="dropout" @click="dropOut(index)">移出队伍</el-button>
                                    </td>
                                    <!-- v-if="detailData.membersID[index]!=detailData.captainID" -->
                                    <!-- <td v-if="iscaptain==true">踢出队伍</td> -->
                                </tr>
                                </template>
                                <!-- <tr class='table_th1'><td valign="middle">{{detailData.members[0]}}</td></tr>
                                <tr class='table_th2'><td valign="middle">2</td></tr>
                                <tr class='table_th1'><td valign="middle">3</td></tr> -->
                            </table>
                        </td>
                        <!-- <td></td> -->
                    </tr>  
                </table>    
            </div>
        </el-tab-pane>
        <el-tab-pane label="加入队伍">
            <el-table
            :data="tableData.slice((currentPage-1)*pagesize,currentPage*pagesize)" 
            stripe
            style="width: 100%">
             <el-table-column
            label="#"
            min-width="5%">
                <template slot-scope="scope">
                    {{scope.$index+(currentPage-1)*pagesize+1}}
                </template>
            </el-table-column>
            <el-table-column
            prop="teamname"
            label="队伍名称"
            min-width="15%">
            </el-table-column>
            <el-table-column
            prop="captain"
            label="队长"
            min-width="12%">
            </el-table-column>
            <el-table-column
            prop="description"
            label="队伍简介"
            min-width="28%"
            v-if="mobile==false"
            >
            </el-table-column>
            <el-table-column
            prop="members"
            min-width="15%"
            align="center"
            v-if="mobile==false"
            label="队伍成员">
                <template slot-scope="scope">
                    <!-- <table style="width:100%;background:#f5f7fa;"frame=void> -->
                        <span v-for="(x,index) in scope.row.members"      style="border-bottom:0px solid ;text-align:center;">
                            |{{x}}|<span v-if="index<(scope.row.members.length-1)"></span><br/>
                        </span>
                    <!-- </table> -->
                </template>
            </el-table-column>
            <el-table-column
            min-width="25%"
            align="center"
            label="操作">
                <template slot-scope="scope">
                    <el-button size="small" type="primary" @click="joinTeam(scope.$index+(currentPage-1)*pagesize)">加入队伍</el-button>
                </template>
            </el-table-column>
            </el-table> 
            <el-pagination
            @current-change="handleCurrentChange"
            :current-page="currentPage"
            background
            :page-size="pagesize"
            layout="prev, pager, next"
            :total="tableData.length">
            </el-pagination>
        </el-tab-pane>
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
console.log(id);

export default {
  name: "team",
  data() {
    // console.log(2);
    return {
      inteam: false, //是否在队伍中  影响“我的队伍”显示界面
      iscaptain: true, //该成员是否为队长  影响“我的队伍”是否出现踢人选项
      team_id:null,
      mobile:false,
      dialogFormVisible: false, //创建队伍对话框
      idx:null,//自己的学号
      isediting:true,//是否正在修改
      form: {
        teamname: "",
        description: "",
        // date1: '',
        // date2: '',
        delivery: false
        // type: [],
        // resource: '',
        // desc: ''
      },
      rules: {
        teamname: [
          { required: true, message: "请输入队伍名称", trigger: "blur" },
          { min: 1, max: 10, message: "长度在 1 到 10 个字", trigger: "blur" }
        ],
        description: [
          { required: true, message: "请输入队伍简介", trigger: "blur" },
          { min: 1, max: 50, message: "长度在 1 到 50 个字", trigger: "blur" }
        ]
      },
      formLabelWidth: "100px", //对话框宽度
      //若当前用户已有队伍，则其队伍信息
      detailData: {
        teamname: "划水大法好",
        captain: "萌新0号",
        invitecode: "1z2x3c4v",
        description: "我们最爱划水",
        members: ["好人", "萌新"]
      },
      currentPage: 1, //初始页
      pagesize: 5, //    每页的数据
      //加入队伍中所有队伍信息
      // tableData: [{
      //id:0,
      //     teamname:"划水大法好",
      //     captain:"萌新0号",
      //     description:"我们最爱划水",
      //     members:['好人','萌新']
      // }, ]
      tableData: [
        {
          teamid: 0,
          teamname: "划水大法好",
          captain: "萌新0号",
          description: "我们最爱划水",
          members: ["好人", "萌新"]
        },
        {
          teamid: 0,
          teamname: "划水大法好",
          captain: "萌新0号",
          description: "我们最爱划水",
          members: ["好人", "萌新", "蒟蒻"]
        }
      ]
    };
  },

  created: function() {
    this.idx=id;
    if(window.screen.width<768)
    {
      this.mobile=true;
      this.pagesize=7;
    }
    fetch("/api/teams", {
      method: "GET",
      headers: {
        "Content-Type": "application/json",
        "x-access-token":JSON.stringify({"token":token,"id":id,"username":username,"auth":true})
      }
    })
      .then(response => {
        console.log(response.status);
        if (response.ok) {
          return response.json();
        } else if (response.status == "401") {
          this.$message.error("token失效，请重新登录！");
        }
      },error=>
      {
        this.$message.error("获取队伍信息失败！")
      })
      .then(res => {
        this.tableData = res;
        for (var i = 0; i < this.tableData.length; i++) {
          if (id == this.tableData[i].captainID) {
            console.log(id);
            this.iscaptain = true;
            this.inteam = true;
            this.detailData["teamname"] = this.tableData[i]["teamname"];
            this.detailData["captain"]= this.tableData[i]["captain"];
            this.detailData["captainID"]= this.tableData[i]["captainID"];
            this.detailData["invitecode"] = this.tableData[i]["invitecode"];
            this.detailData["description"] = this.tableData[i]["description"];
            this.detailData["members"] = this.tableData[i]["members"];
            this.detailData["membersID"] = this.tableData[i]["membersID"];
            this.team_id=this.tableData[i]["teamid"]

          } else {
            for (var j = 0; j < this.tableData[i].membersID.length; j++) {
              if (id ==this.tableData[i].membersID[j]) {
                this.inteam = true;
                this.iscaptain=false;//
                this.detailData["teamname"] = this.tableData[i]["teamname"];
                this.detailData["captain"] = this.tableData[i]["captain"];
                 this.detailData["captainID"]= this.tableData[i]["captainID"];
                this.detailData["invitecode"] =this.tableData[i]["invitecode"];
                this.detailData["description"] = this.tableData[i]["description"];
                this.detailData["members"] = this.tableData[i]["members"];
                this.detailData["membersID"] = this.tableData[i]["membersID"];
                this.team_id=this.tableData[i]["teamid"]
              }
            }
          }
        }
      });
  },

  methods: {
    createDialog() {
      this.dialogFormVisible = true;
      this.form.teamname = "";
      this.form.description = "";
    },
    createTeam() {
      // console.log(this.form['teamname']);
      // console.log(this.form['description']);
      if (
        this.form["teamname"].length <= 10 &&
        this.form["teamname"].length > 0
      ) {
        //
        if (
          this.form["description"].length <= 50 &&
          this.form["description"].length > 0
        ) {
          fetch("/api/teams",{
            method: "POST",
            headers: {
            "Content-Type": "application/json",
            "x-access-token":JSON.stringify({"token":token,"id":id,"username":username,"auth":true})
            },
            body: JSON.stringify({
                "teamname":this.form["teamname"],
                "description":this.form["description"]
            })
          })
            .then(response => {
              console.log(response.status);
              if (response.ok) {
                return response.json();
              } else if (response.status == "401") {
                this.$message.error("token失效，请重新登录！");
                if(token!=null)
                {
                    delCookie("token")
                    delCookie("id")
                    delCookie("username")
                    token=null
                    username=null
                    id=null
                    setTimeout(() => {
                      window.location="https://teamstyle.eesast.com/team";
                    }, 100)
                }
              } else if (response.status == "409"){
                this.$message.error("您已加入本队伍或队伍名冲突！");
              } else if (response.status == "422"){
                  this.$message.error("队伍信息重要字段缺失！")
              } else if (response.status == "500"){
                  this.$message.error("500服务器故障！")
              } 
            },error=>
            {
              this.$message.error("创建队伍失败！")
            })
            .then(res => {
              this.detailData.invitecode = res["invitecode"]
              this.iscaptain=true
              this.inteam=true
              this.$message.success("创建队伍成功!")
              // delCookie("token")
              // delCookie("username")
              // delCookie("id")
              setTimeout(() => {
                // this.$router.push({path: '/Login'})
                window.location="https://teamstyle.eesast.com/team";
              }, 100)
            });
            this.dialogFormVisible = false;
        } else if (this.form["description"].length == 0) {
          this.$message.error("请输入队伍简介");
        } else {
          this.$message.error("队伍简介过长");
        }
      } else if (this.form["teamname"].length == 0) {
        //显示‘请输入队伍名称’
        this.$message.error("请输入队伍名称");
      } //显示‘队伍名称过长’
      else {
        this.$message.error("队伍名称过长");
      }
    },
    onCopy() {
      this.$message.success("复制成功");
    },
    onError() {
      this.$message.error("复制失败");
    },
    dropOut(index) {
      if(this.iscaptain==true)
      {
          this.$confirm('是否确定要将队友&nbsp;&nbsp;&nbsp;<span style="color:red">' +this.detailData.members[index] +"</span>&nbsp;&nbsp;&nbsp;请出队伍?","提示",
          {
            confirmButtonText: "确定",
            cancelButtonText: "取消",
            dangerouslyUseHTMLString: true,
            type: "warning"
          })
          .then(() => {
              //some change *****

              console.log(this.detailData.membersID[index])
              var FETCH_URL="/api/teams/"+this.team_id+"/members/"+this.detailData.membersID[index];
              
            fetch(FETCH_URL, {
            method: "DELETE",
            headers: {
            "Content-Type": "application/json",
            "x-access-token":JSON.stringify({"token":token,"id":id,"username":username,"auth":true})
            },
            }).then(response => {
              console.log(response.status);
              if(response.status=="204")
              {
                this.$message.success("删除队员成功!");
                setTimeout(() => {
                  window.location="https://teamstyle.eesast.com/team";
                  }, 100)
              }
              if (response.ok) {
                return ;
              } else if (response.status == "401") {
                this.$message.error("token失效或权限不足！");
                if(token!=null)
                {
                    delCookie("token")
                    delCookie("id")
                    delCookie("username")
                    token=null
                    username=null
                    id=null
                    setTimeout(() => {
                    window.location="https://teamstyle.eesast.com/team";
                    }, 100)
                }
              } else if (response.status == "404"){
                this.$message.error("队伍或成员不存在！");
              } else if (response.status == "400"){
                this.$message.error("不能踢出自己！");
              } else if (response.status == "500"){
                this.$message.error("Internal proxy error!");
              } 
            },error=>{
              this.$message.error("踢出成员失败！")
            }).then(res => {
              // this.iscaptain=false
              // this.inteam=false
              // this.team_id=null
              // token=null
              // username=null
              // id=null
              // delCookie("token")
              // delCookie("username")
              // delCookie("id")
              setTimeout(() => {
                // this.$router.push({path: '/Login'})
                window.location="https://teamstyle.eesast.com/team";
              }, 100);
            })



              //********8drop */


          }).catch(() => {
            this.$message({
              type: "info",
              message: "已取消删除"
            })
          });
      }
      else 
      {
          this.$message.error("您不是队长，无权进行此操作！")
      }
    },
    joinTeam(index)//加入第几个队伍 
      {
      console.log(this.tableData[index].teamid)//为加入队伍的标号
      this.$prompt("请输入邀请码", "提示", {
        confirmButtonText: "确定",
        cancelButtonText: "取消"
        // inputPattern: /[\w!#$%&'*+/=?^_`{|}~-]+(?:\.[\w!#$%&'*+/=?^_`{|}~-]+)*@(?:[\w](?:[\w-]*[\w])?\.)+[\w](?:[\w-]*[\w])?/,
        // inputErrorMessage: '邮箱格式不正确'
      })
        .then(({ value }) => {
            var want_teamid = this.tableData[index].teamid
            var FETCH_URL="/api/teams/"+want_teamid+"/members"
            fetch(FETCH_URL, {
            method: "POST",
            headers: {
            "Content-Type": "application/json",
            "x-access-token":JSON.stringify({"token":token,"id":id,"username":username,"auth":true})
            },
            body: JSON.stringify({
                "invitecode":value
            })
            })
            .then(response => {
              console.log(response);
              console.log(response.status);
              if(response.status=="201")
              {
                this.$message.success("加入队伍成功!");
                setTimeout(() => {
                  window.location="https://teamstyle.eesast.com/team";
                  }, 100)
              }
              if (response.ok) {
                return response.json();
              } else if (response.status == "401") {
                this.$message.error("token失效或权限不足！");
                if(token!=null)
                {
                    delCookie("token")
                    delCookie("id")
                    delCookie("username")
                    token=null
                    username=null
                    id=null
                    setTimeout(() => {
                      window.location="https://teamstyle.eesast.com/team";
                    }, 100)
                }
              } else if (response.status == "422"){
                this.$message.error("缺少邀请码字段！");
              } else if (response.status == "403"){
                this.$message.error("邀请码错误！");
              } else if (response.status == "409"){
                this.$message.error("队伍人数超过上限或您已经在队伍中！");
              } else if (response.status == "404"){
                this.$message.error("队伍不存在！");
              }
            },error=>
            {
               this.$message.error("加入队伍失败！")
            })
            .then(res => {
              // this.iscaptain=false
              // this.inteam=true
              // this.team_id=null
              // token=null
              // username=null
              // id=null
              // delCookie("token")
              // delCookie("username")
              // delCookie("id")
              setTimeout(() => {
                // this.$router.push({path: '/Login'})
                window.location="https://teamstyle.eesast.com/team";
              }, 100);
            })
        })
        .catch(() => {
          this.$message({
               type: 'info',
               message: '取消加入队伍'
           });
        });
    },
    clearTeam() {
        if(this.iscaptain==true)
        {
           this.$confirm('是否确定解散该队伍',"提示",
          {
            confirmButtonText: "确定",
            cancelButtonText: "取消",
            dangerouslyUseHTMLString: true,
            type: "warning"
          }).then(()=>{
            // this.$message({
            //     type: 'info',
            //     message: '您正在解散队伍，请谨慎操作！'
            // });
            var FETCH_URL="/api/teams/"+this.team_id
              fetch(FETCH_URL, {
              method: "DELETE",
              headers: {
              "Content-Type": "application/json",
              "x-access-token":JSON.stringify({"token":token,"id":id,"username":username,"auth":true})
              }
            })
            .then(response => {
              console.log(response.status);
              if(response.status=="204")
              {
                this.$message.success("删除队员成功!");
                setTimeout(() => {
                  window.location="https://teamstyle.eesast.com/team";
                  }, 100)
              }
              if (response.ok) {
                return response.json()
              } 
              else if (response.status == "401") {
                this.$message.error("token失效或权限不足！");
                if(token!=null)
                {
                    delCookie("token")
                    delCookie("id")
                    delCookie("username")
                    token=null
                    username=null
                    id=null
                    setTimeout(() => {
                      window.location="https://teamstyle.eesast.com/team";
                    }, 100)
                }
              } 
              else if (response.status == "404"){
                this.$message.error("队伍不存在！");
              }
              else if (response.status == "500"){
                this.$message.error("Internal proxy error！");
              }
            },error=>
            {
              this.$message.error("删除队伍失败！")
            })
            .then(res => {
              // this.iscaptain=false
              // this.inteam=false
              // this.team_id=null
              // token=null
              // username=null
              // id=null
              // delCookie("token")
              // delCookie("username")
              // delCookie("id")
              setTimeout(() => {
                // this.$router.push({path: '/Login'})
                window.location="https://teamstyle.eesast.com/team";
              }, 100);
            });
        }).catch(()=>{})
        }
        else{
            this.$message.error("您不是队长，没有解散队伍的权限！")
        }
        
    }, 
    exitTeam() {
        if(this.inteam==true&&this.iscaptain==false)
        {
           this.$confirm('是否确定要退出队伍',"提示",
            {
              confirmButtonText: "确定",
              cancelButtonText: "取消",
              dangerouslyUseHTMLString: true,
              type: "warning"
            }
          ).then(()=>{
            // this.$message({
            //     type: 'info',
            //     message: '您正在退出队伍，请谨慎操作！'
            // });
            // if(this.team)
            var FETCH_URL="/api/teams/"+this.team_id+"/members"
            fetch(FETCH_URL, {
            method: "DELETE",
            headers: {
            "Content-Type": "application/json",
            "x-access-token":JSON.stringify({"token":token,"id":id,"username":username,"auth":true})
            }
          })
            .then(response => {
              console.log(response.status);
              if(response.status=="204")
              {
                this.$message.success("删除队员成功!");
                setTimeout(() => {
                  window.location="https://teamstyle.eesast.com/team";
                  }, 100)
              }
              if (response.ok) {
                return response.json();
              } else if (response.status == "401") {
                this.$message.error("token失效或权限不足！");
                if(token!=null)
                {
                    delCookie("token")
                    delCookie("id")
                    delCookie("username")
                    token=null
                    username=null
                    id=null
                    setTimeout(() => {
                      window.location="https://teamstyle.eesast.com/team";
                    }, 100)
                }
              } else if (response.status == "404"){
                this.$message.error("队伍或队伍成员不存在！");
              } else if (response.status == "400"){
                this.$message.error("不能试图删除队长");
              } else if (response.status == "500"){
                this.$message.error("Internal proxy error!");
              }
            },error=>
            {
              this.$message.error("退出队伍失败！")
            })
            .then(res => {
              // this.iscaptain=false
              // this.inteam=false
              // this.team_id=null
              // token=null
              // username=null
              // id=null
              // delCookie("token")
              // delCookie("username")
              // delCookie("id")
              setTimeout(() => {
                // this.$router.push({path: '/Login'})
                // window.location="https://teamstyle.eesast.com/team";           
              }, 100);
            });
          }).catch(()=>{})
        }
        else if(this.inteam==true&&iscaptain==true){
            this.$message.error("您是队长，不可退出自己的队伍，请选择解散队伍或删除队伍成员！")
        }
        else if(this.inteam==false)
        {
            this.$message.error("您尚且未加入队伍中，请先加入队伍！")
        }
    }, 
    edit_description() {
        if(this.isediting==true)
        {
          this.isediting=false;
          return ;
        }
        if(this.iscaptain==true)
        {
            var FETCH_URL="/api/teams/"+this.team_id
            fetch(FETCH_URL, {
            method: "PUT",
            headers: {
            "Content-Type": "application/json",
            "x-access-token":JSON.stringify({"token":token,"id":id,"username":username,"auth":true})
            },
            body: JSON.stringify({
                "description":this.detailData["description"]
            })
         })
            .then(response => {
              console.log(response.status);
              if (response.ok) {
                // return response.json();
                return ;
              } else if (response.status == "401") {
                this.$message.error("token失效或权限不足！");
                if(token!=null)
                {
                    delCookie("token")
                    delCookie("id")
                    delCookie("username")
                    token=null
                    username=null
                    id=null
                    setTimeout(() => {
                      window.location="https://teamstyle.eesast.com/team";
                    }, 100)
                }
              } else if (response.status == "400"){
                this.$message.error("您不是队长或权限不足！");
              } else if (response.status == "404"){
                this.$message.error("队伍不存在！");
              } else if (response.status == "500"){
                this.$message.error("Internal proxy error!");
              } 
            },error=>{
              this.$message.error("加入队伍失败！")
            })
            .then(()=>{
              // this.iscaptain=false
              // this.inteam=true
              // this.team_id=null
              // token=null
              // username=null
              // id=null
              // this.isediting=true;
              // delCookie("token")
              // delCookie("username")
              // delCookie("id")
              setTimeout(() => {
                // this.$router.push({path: '/Login'})
                window.location="https://teamstyle.eesast.com/team";
              }, 100);
            })
        }
        else{
            this.$message.error("您不是队长，没有权限")
        }
    }, 
    handleCurrentChange: function(currentPage) {
      this.currentPage = currentPage;
      // console.log(this.currentPage)  //点击第几页
    }
  }
};


function delCookie(name)
{
var exp = new Date();
exp.setTime(exp.getTime() - 1);
var cval=getCookie(name);
if(cval!=null)
document.cookie= name + "="+cval+";expires="+exp.toGMTString()+";path=/";
}

</script>

<style>
.svg {
  position: relative;
  padding-right: 12px;
  top: 3px;
  width: 20px;
  height: 20px;
  /* padding: 12px; */
}
#team_contain {
  width: 100%;
  min-height: 80%;
  margin-bottom: 50px;
}
#team_card {
  position: relative;
  width: 70%;
  left: 15%;
  margin-top: 2%;
}
#create_team {
  width: 100%;
  position: relative;
  /* padding:20px; */
}
.create_team {
  position: relative;
  margin-top: 20px;
}
#create_team_dialog {
  width: 100%;
  /* position: relative; */
}
#detail_table {
  padding: 12px 0;
  min-width: 0;
  width: 100%;
  -webkit-box-sizing: border-box;
  box-sizing: border-box;
  text-overflow: ellipsis;
  vertical-align: middle;
  position: relative;
  text-align: left;
  color: #909399;
  border-collapse: collapse;
}
#detail_table .el-button {
  width: 90px;
  margin-right: 5px;
}
#detail_table tr {
  /* border:1px solid red; */
  width: 100%;
  padding: 12px;
  border-bottom: 1px solid #f5f7fa;
}
#detail_table tr th {
  padding: 12px;
}
#detail_table tr th i {
  padding-right: 12px;
}
#detail_table tr td {
  position: relative;
  /* padding:12px; */
  vertical-align: middle;
  color: black;
  /* left:200px; */
}
.table_th2 {
  background-color: #f5f7fa;
}
.table_th1 {
  background-color: white;
}
#members_table {
  width: 100%;
}
#members_table .table_th2 {
  background-color: #f5f7fa;
  padding-top: 10px;
  height: 50px;
  position: relative;
}
#members_table .table_th1 {
  background-color: white;
  padding-top: 10px;
  height: 50px;
  position: relative;
}
#copyinvitecode {
  position: relative;
  /* left:50px; */
  float: right;
  /* text-align: right; */
}
.dropout {
  position: relative;
  /* left:50%; */
  float: right;
}
@media screen and (max-width:720px) {
#team_contain {
  width: 100%;
  min-height: 80vh;
  margin-bottom: 50px;
}
#team_card {
  position: relative;
  width: 90%;
  left: 5%;
  margin-top: 12vh;
}
#team_contain .el-dialog
{
  width:100%;
}
.el-message-box
{
  position: fixed;
  
  width:90vw!important;;
  /* display: block; */
  top:40vh;
  left:5vw;
}
}
</style>