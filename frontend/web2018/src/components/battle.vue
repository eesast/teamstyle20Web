<template>
<div id="battle_contain">
    <div class="code_content">
        <el-card shadow="always" style="text-align:left;">
            <h4>提交代码</h4>
            <h5><i class="el-icon-info"></i>请选择对应的职业后上传代码</h5>
            <!-- <el-upload
            class="upload-demo"
            action="x"
            :show-file-list="true" 
            :limit="4"
            :on-success="uploadSuccess"
            :on-error="uploadError" 
            >
            <el-button size="small" type="primary" icon="el-icon-upload">点击上传</el-button>
            </el-upload> -->
            
            <el-select v-model="value" placeholder="请选择您的职业" style="margin-bottom:10px;">
              
            <el-option
              v-for="item in options"
              :key="item.value"
              :label="item.label"
              :value="item.value">
            </el-option>
            
            </el-select>
           
            <el-upload
            ref="upload"
            class="upload-demo"
            action=""
            v-loading="loading_upload"
            element-loading-text="文件上传中"
            element-loading-spinner="el-icon-loading"
            :http-request="myUpload"
            :on-preview="handlePreview"
            :on-remove="handleRemove"
            :before-remove="beforeRemove"
            :limit="1"    
            :on-exceed="handleExceed"
            :file-list="fileList">
   
             <div slot="tip" class="el-upload__tip">只能上传.cpp文件
               <br/><span style="font-weight:600;">代码有效性:</span><br/>
               职业1: <span v-if="valid%2==1">有效</span><span v-else>无效</span><br/>
               职业2: <span v-if="(valid>>1)%2==1">有效</span><span v-else>无效</span><br/>
               职业3: <span v-if="(valid>>2)%2==1">有效</span><span v-else>无效</span><br/>
               职业4: <span v-if="(valid>>3)%2==1">有效</span><span v-else>无效</span><br/>
             </div>
             <el-button size="small" type="primary" icon="el-icon-upload">点击上传</el-button>
             <!-- <el-button size="small" type="primary" icon="el-icon-loading" v-if="loading_upload==true" disabled>文件上传中</el-button> -->
            </el-upload>
            <!-- <el-input type="file" @onchange="jsReadFiles(this.files)"/> -->
            <h5><i class="el-icon-info"></i>系统仅保留最后一次上传的结果</h5>
            <h5><span style="color:red"><i class="el-icon-info"></i>代码与对战截止日期:{{finaldate.toString()}}</span></h5>
            <br/>
            <h4>发起对战</h4>
            <h5><i class="el-icon-info"></i>仅可以选择代码<span style="color:red">有效</span>的队伍进行对战</h5>
            <h5><i class="el-icon-info"></i><span style="color:red">有效</span>指该队伍各职业代码都已经提交且都编译通过</h5>
            <el-button size="small"type="danger" style="padding-left:20px;padding-right:20px;" @click="dialogTableVisible = true" v-if="loading_fight==false">发起对战</el-button>
            <el-button size="small"type="danger" style="padding-left:20px;padding-right:20px;"  v-if="loading_fight==true" icon="el-icon-loading" disabled="true">对战中</el-button>
            <h5>今日对战次数:{{10-battle_time}}</h5>
            <h5>剩余对战次数:{{battle_time}}</h5>
            <br/>
            <h4>查看代码</h4>
            <h5><i class="el-icon-info"></i>可以下载已上传的代码</h5>
            <h5><i class="el-icon-info"></i>请选择对应的职业后下载代码</h5>
            <el-select v-model="codevalue" placeholder="请选择您的职业" style="margin-bottom:10px;">
              
            <el-option
              v-for="item in codeoptions"
              :key="item.value"
              :label="item.label"
              :value="item.value">
            </el-option>
            
            </el-select>
            <el-row>
            <!-- <el-button size="small"type="primary"icon="el-icon-view" @click="lookcode()">查看代码</el-button> -->
            <el-button size="small"type="success"icon="el-icon-download" style="margin:2px;"  @click="downloadcode()">点击下载</el-button>
            </el-row>
        </el-card>
    </div>    
    <div class="empty_content"></div>
    <div class="battle_content">
        <el-card shadow="always" style="width:100%;">
            <el-table
            :data="tableData"
            :default-sort = "{prop: 'score', order: 'descending'}"
            stripe
            style="width: 100%">
            <el-table-column
            label="#"
            min-width="10%">
                <template slot-scope="scope">{{scope.$index+1}}</template>
            </el-table-column>
            <el-table-column
            prop="teamname"
            label="队伍名称"
            sortable
            min-width="25%">
            </el-table-column>
            <el-table-column
            prop="captain"
            label="队长"
            sortable
            min-width="20%">
            </el-table-column>
            <el-table-column
            prop="score"
            label="得分"
            sortable
            min-width="15%">
            </el-table-column>
            
            </el-table>
        </el-card>
    </div>
    <el-dialog title="选择对战队伍" :visible.sync="dialogTableVisible" id="battle_dialog" >
      <el-checkbox-group v-model="checkList" @change="handleChecked" :min="0" :max="15">
      <el-row v-for="index in tableData.length" >
      <template v-if="index%4==1">
      <el-col :span="6" v-if="tableData[index-1].valid==15"><el-checkbox :label="tableData[index-1].teamid" style="margin:3px;" :disabled="tableData[index-1].teamid==teamid">{{tableData[index-1].teamname}}</el-checkbox></el-col>
      <el-col :span="6" v-if="index<tableData.length&&tableData[index].valid==15"><el-checkbox :label="tableData[index].teamid" style="margin:3px;" :disabled="tableData[index].teamid==teamid">{{tableData[index].teamname}}</el-checkbox></el-col>
      <el-col :span="6" v-if="index+1<tableData.length&&tableData[index+1].valid==15"><el-checkbox :label="tableData[index+1].teamid" style="margin:3px;" :disabled="tableData[index+1].teamid==teamid">{{tableData[index+1].teamname}}</el-checkbox></el-col>
      <el-col :span="6" v-if="index+2<tableData.length&&tableData[index+2].valid==15"><el-checkbox :label="tableData[index+2].teamid" style="margin:3px;" :disabled="tableData[index+2].teamid==teamid">{{tableData[index+2].teamname}}</el-checkbox></el-col>
      </template>
      </el-row>
      <!-- <el-row>
      <el-checkbox label="复选框 B"></el-checkbox>
      </el-row>
      <el-checkbox label="复选框 C"></el-checkbox>
      <el-checkbox label="禁用" disabled></el-checkbox>
      <el-checkbox label="选中且禁用" disabled></el-checkbox> -->
     </el-checkbox-group>
     <h2>AI数量:<el-input-number v-model="AInum"  :min="0" :max="16-checkList.length" size="small" style="margin-left:5px;"></el-input-number></h2>
     <el-button type="danger" size="small" @click="start_fight()">开始对战</el-button>
    </el-dialog>
    <div class="empty_content"></div>
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
export default {
    name: 'battle',
    data() {
        return {
            fileList: [],            // currentPage:1, //初始页
            // pagesize:100,    //    每页的数据
            dialogTableVisible:false,
            isIndeterminate: true,
            loading_upload:false,
            loading_fight:false,
            checkList: [],
            AInum:0,//AI人数
            teamid:0,//队伍id
            valid:0,//有效
            battle_time:10,//
            tableData: [],
            finaldate:null,
            //[{
            // teamname:'划水萌新',
            // captain:'萌新1号',
            // teamid:0,
            // score:20,
            // battle_time:0,
            // valid:15,//是否有效
            // }],//
            options: [{
              value: 'code0',
              label: '职业1'
            }, {
              value: 'code1',
              label: '职业2'
            }, {
              value: 'code2',
              label: '职业3'
            }, {
              value: 'code3',
              label: '职业4'
            }],
            value: '',
            codeoptions: [{
              value: '0',
              label: '职业1'
            }, {
              value: '1',
              label: '职业2'
            }, {
              value: '2',
              label: '职业3'
            }, {
              value: '3',
              label: '职业4'
            }],
            codevalue: ''

        }
    },
    created: function()
    {
      this.finaldate=new Date("2019-05-16T12:00:00+08:00");
      fetch('/api/global',{
        method:'GET',
        headers:{
        "Content-Type": "application/json",
        "x-access-token":JSON.stringify({"token":token,"id":id,"username":username,"auth":true})
        }
      }).then(response=>{
         if(response.ok)
         {
           return response.json();
         }
         else throw 'bad'
      }).then(res=>{
          this.finaldate=new Date(res.submission_end);
      }).catch(()=>{this.$message.error('服务器无响应!')})

        fetch('/api/teams/0/members/'+id,
            {
                method:'GET',
                headers: {
                "Content-Type": "application/json",
                "x-access-token":JSON.stringify({"token":token,"id":id,"username":username,"auth":true})
                }
            }).then(response=>
            {
                if(response.status=="404")
                {
                    //没有队伍
                    this.$message.error('您没有加入队伍！');
                   
                    
                    // throw '';
                }
                else if (response.status=="200") {
                return response.json();
                } 
                else if (response.status == "401") {
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
                      window.location="https://teamstyle.eesast.com/login";
                    }, 100)
                }
                }
                else
                {
                    this.$message.error("服务器暂时无法响应！");
                }
                throw 'bad';
            },error=>
            {
                this.$message.error("加载失败，请稍后刷新页面重试！")
            }).then(res=>
            {
                  this.teamid=res.teamid;
                  this.battle_time=res.battle_time;//对战次数
                  this.valid=res.valid;//有效
                  if(this.valid==15)
                  {
                    this.checkList.push(this.teamid);
                  }
            }).catch(()=>{

            })




        fetch("/api/teams_cached", {
          method: "GET",
          headers: {
            "Content-Type": "application/json",
            "x-access-token":JSON.stringify({"token":token,"id":id,"username":username,"auth":true})
          }
        })
          .then(response => {
            //console.log(response.status);
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
                    window.location="https://teamstyle.eesast.com/login";
                  }, 100)
              }
            }
            else
            {
              // this.$message.error("获取队伍信息失败！")
              throw 'bad';
            }
          })
          .then(res => {
            // this.tableData = res;
            this.tableData=[];
            for(var i=0;i<res.length;i++)
            {
              if(res[i].valid==15)
              {
                this.tableData.push(res[i]);
              }
            }
            // for (var i = 0; i < this.tableData.length; i++) {
            //   for (var j = 0; j < this.tableData[i].membersID.length; j++) {
            //     // if(id==this.tableData[i].membersID[j])
            //     // {
            //     //   if(this.tableData[i].valid==15)
            //     //   {
            //     //     this.checkList.push(this.teamid);
            //     //   }
            //     // }
            //   }
            // }
          },error=>{
            this.$message.error("获取队伍信息失败！")
          })
    },
    methods: {
      jsReadFiles(file)
      {
          var file = files[0];
          //console.log(file);
      },
      lookcode()//查看代码
      {
          fetch('/api/codes/teams/'+this.teamid+'/'+this.codevalue,
          {
            method:'GET',
            headers:{
              "content-type": "application/octet-stream",
              "x-access-token":JSON.stringify({"token":token,"id":id,"username":username,"auth":true})
            }
          }).then(res=>{
              //console.log(res);
              this.$confirm(res,"浏览",
              {
                confirmButtonText: "确定",
                // cancelButtonText: "取消",
                dangerouslyUseHTMLString: true,
                type: "warning"
              }).then(()=>{}).catch(()=>{})
          })
      },
      downloadcode()//下载代码
      {
          fetch('/api/codes/teams/'+this.teamid+'/'+this.codevalue,
          {
            method:'GET',
            headers:{
              "content-type": "application/octet-stream",
              "x-access-token":JSON.stringify({"token":token,"id":id,"username":username,"auth":true})
            }
          }).then(res => res.blob().then(blob => { 
              var a = document.createElement('a'); 
              var url = window.URL.createObjectURL(blob);   // 获取 blob 本地文件连接 (blob 为纯二进制对象，不能够直接保存到磁盘上)
              var filename = res.headers.get('Content-Disposition'); 
              a.href = url; 
              a.download = "职业"+(parseInt(filename[filename.length-5])+1)+'.cpp'; 
              a.click(); 
              window.URL.revokeObjectURL(url);
          })).catch(()=>{
            this.$message.error('文件不存在!');
          })
      },
      myUpload(content)
      {
          console.log(this.finaldate);
          var nowdate=new Date();
          if(nowdate>this.finaldate)
          {
            this.$message.error('代码上传已截止!');
            this.fileList=[];
            return ;
          }
          if(this.value=='')
          {
            this.$message.error('请先选择职业!');
            this.fileList=[];
            return ;
          }
          this.$confirm('是否确定上传?<br/><span style="font-size:10px;">若之前已经上传过该职业的代码，上传代码会覆盖之前已有的代码</span>',"提示",
          {
            confirmButtonText: "确定",
            cancelButtonText: "取消",
            dangerouslyUseHTMLString: true,
            type: "warning"
          })
          .then(() => {
            // //console.log(content);
            this.loading_upload=true;
            //console.log(content.file);
            // //console.log(this.fileList);
            //console.log(typeof content.file);
            // //console.log(this.value.toString());

            // var filepath="C:/Users/nikelong/Desktop/1.cpp";
      
            // var file = new File(filepath,"ok");
            // //console.log(file);

            var fileobj=content.file;
            // var URL=;
            // var form=new FormData();
            // form.append(this.value.toString(),content.file);

            var form = new FormData();
            form.append(this.value.toString(),content.file);
            //console.log(form.get('code0'));



          fetch("/api/codes/teams/"+this.teamid,{
            method:'POST',
            headers:{
              // "content-type": "multipart/form-data",
              //  "Content-Type": "multipart/form-data",
              "x-access-token":JSON.stringify({"token":token,"id":id,"username":username,"auth":true})
            },
            body:form,
          }).then(response=>{
            this.loading_upload=false;
            this.fileList=[];
            if(response.status=="205")
            {
              this.$message.success('上传成功!');      
              return response.json();
              
            }
            else if(response.status=="401")
            {
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
                    window.location="https://teamstyle.eesast.com/login";
                  }, 100)
              }
            }
            else if(response.status=="403")
            {
              this.$message.error('不在系统开放时间内!');
            }
            else 
            {
              this.$message.error('上传失败!');
            }
          }).then(res=>{
                //console.log(res);
                this.$alert(res[0].status+'<br/>'+res[0].information, '编译结果', {
                confirmButtonText: '确定',
                dangerouslyUseHTMLString: true,
                callback: action => {
                  this.valid|=1<<parseInt(this.value[this.value.length-1]);
                  // this.$message({
                  //   type: 'info',
                  //   message: `action: ${ action }`
                  // });
                  // window.location="https://teamstyle.eesast.com/battle";
                }
              });

            }).catch(()=>{this.loading_upload=false;})

          }).catch(()=>{
            this.fileList=[];
          })
      },
      handleRemove(file, fileList) {
        //console.log(file, fileList);
      },
      handlePreview(file) {
        //console.log(file);
      },
      handleExceed(files, fileList) {
        this.$message.warning(`当前限制选择 1 个文件，本次选择了 ${files.length} 个文件，共选择了 ${files.length + fileList.length} 个文件`);
      },
      beforeRemove(file, fileList) {
        return this.$confirm(`确定移除 ${ file.name }？`);
      },
        // uploadSuccess(res){
        //     if(res.status==1){
        //       this.$message({
        //         message: '恭喜你，上传成功',
        //         type: 'success'
        //       });
        //     }
        //     else if(res.status==0){
        //       this.$message({
        //         message: res.msg,
        //         type: 'warning'
        //       });
        //     }
        //     else{
        //       this.$message.error('上传失败，请重新上传');
        //     }
        //   },
        //   uploadError(){
        //     this.$refs.upload.clearFiles();
        //     this.$message.error('上传失败，请重新上传');
        //   },
          // handleCurrentChange: function(currentPage){
          //       this.currentPage = currentPage;
          //       // //console.log(this.currentPage)  //点击第几页
        // },
        handleChecked(value) {
        let checkedCount = value.length;
        this.isIndeterminate = checkedCount > 0 && checkedCount < this.tableData.length;
      },
        start_fight()  //发起一场战斗
        {
          //关闭对战
          //this.$message.error('对战截止时间:2019/4/25 12:00');
          //return ;
		  var nowdate=new Date();
          if(nowdate>this.finaldate)
          {
            this.$message.error('对战已截止!');
            return ;
          }
          this.dialogTableVisible=false;//隐藏对话框
          //console.log(this.checkList);//勾选的队伍
          if(this.valid!=15)
          {
             this.$alert("对战发起失败，请确保所有职业代码均有效！<br/>职业一:"+((this.valid%2==1)?'有效':'<span style="color:red;">无效</span>')+"<br/>职业二:"
             +(((this.valid>>1)%2==1)?'有效':'<span style="color:red;">无效</span>')+"<br/>职业三:"+(((this.valid>>2)%2==1)?'有效':'<span style="color:red;">无效</span>')+"<br/>职业四:"+(((this.valid>>3)%2==1)?'有效':'<span style="color:red;">无效</span>')
             , '错误', {
              confirmButtonText: '确定',
              dangerouslyUseHTMLString: true,
              // type:"error",
              callback: action => {
                // this.$message({
                //   type: 'info',
                //   message: `action: ${ action }`
                // });
              }
            })
            return ;//不能对战
          }
          else if(this.checkList.length+this.AInum<2)
          {
            this.$message.error("至少需要2支队伍才能参赛!");
            return ;
          }
          this.loading_fight=true;
          //需要根据勾选的队伍，传送一场比赛  checkList里面记录的是选中的队伍的ID，当且仅当el-checkBox的:label绑定的是x.id;   
          //当el-checkBox 的:label绑定的是x.teamname时，checkList记录选中队伍的teamname
          fetch('/api/battle/add'+"?teams=["+this.checkList+"]&AInum="+this.AInum+"&initiator_id="+this.teamid,{
            method:'GET',
            headers:{
                "Content-Type": "application/json",
                "x-access-token":JSON.stringify({"token":token,"id":id,"username":username,"auth":true})
            },
            // body:JSON.stringify({
            //   // "teams":,
            //   // "AInum":,
            //   // "initiator_id":this.teamid
            // })
          }).then(response=>{
            //console.log(response);
            if(response.ok)
            {
              // return response.json();
              // this.$message.success('对战发起成功，请在""')
              this.$alert("对战发起成功，请在<a href='https://teamstyle.eesast.com/personal'>个人中心</a>界面查看对战结果", '提示', {
                confirmButtonText: '确定',
                dangerouslyUseHTMLString: true,
                callback: action => {
                  // this.$message({
                  //   type: 'info',
                  //   message: `action: ${ action }`
                  // });
                }
              })
            }
            else if(response.status=="405")
            {
              this.$message.error('已达今日对战次数上限!');
            }
            else if(response.status=="406")
            {
                this.$message.error('选择的队伍不符合要求');
            }
            else throw 'bad';
          }).then(res=>{

          }).catch(()=>{
              this.$message.error('发起对战失败');
          })
          //以下将传送checkList这些标号的队伍，以及AInum个AI进行对战   *********************

          this.loading_fight=false;
        }
        
    }
}
</script>

<style>
#battle_contain
{
    position: relative;
    width:100%;
    min-height: 70%;
    margin-bottom:50px; 
}
.code_content,.battle_content,.empty_content
{
    display: inline-block;
    vertical-align: top;
    position: relative;
    margin-top:30px;
}
.code_content
{
    width:20%;
    /* min-width:250px; */
    /* min-height: 100%; */
    /* min-height:100%; */
    /* background: black; */
    /* position: relative; */
}
.code_content div.el-card__body
{
  /* min-width:250px;
  overflow:auto; */
}
.empty_content
{
    width:5%;
}
.battle_content
{
    /* left:5%; */
    /* height:100%; */
    /* top:; */
    width:70%;
    left:-2%;
    /* min-height:70%; */
    /* position: relative; */
}
#battle_dialog
{
  width:100%;
  min-width:565px;
  position: fixed;
  top:0%;
  left:0%;
}
#battle_dialog .el-dialog
{
  min-width:620px;
}
#battle_dialog .el-dialog__body,#battle_dialog .el-dialog__header
{
  min-width: 565px;
}
#battle_dialog span.el-checkbox__label
{
  width:100px;
}
@media screen and (max-width:720px) {
.code_content,.battle_content,.empty_content
{
    display: block;
}
.code_content{
  width:100%;
}
.battle_content
{
    /* left:5%; */
    /* height:100%; */
    /* top:; */
    width:100%;
    left:0%;
    /* min-height:70%; */
    /* position: relative; */
}
}
</style>