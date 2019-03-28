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
            :http-request="myUpload"
            :on-preview="handlePreview"
            :on-remove="handleRemove"
            :before-remove="beforeRemove"
            :limit="1"    
            :on-exceed="handleExceed"
            :file-list="fileList">
   
             <div slot="tip" class="el-upload__tip">只能上传.cpp文件</div>
             <el-button size="small" type="primary" icon="el-icon-upload">点击上传</el-button>
            </el-upload>
            <!-- <el-input type="file" @onchange="jsReadFiles(this.files)"/> -->
            <h5><i class="el-icon-info"></i>系统仅保留最后一次上传的结果</h5>
            <h5><span style="color:red"><i class="el-icon-info"></i>代码提交截止日期:3/24 24:00</span></h5>
            <br/>
            <h4>发起对战</h4>
            <el-button size="small"type="danger" style="padding-left:20px;padding-right:20px;" @click="dialogTableVisible = true">发起对战</el-button>
            <h5>今日对战次数:</h5>
            <h5>剩余对战次数:</h5>
            <br/>
            <h4>查看代码</h4>
            <h5><i class="el-icon-info"></i>可以下载已上传的代码</h5>
            <h5><i class="el-icon-info"></i>请选择对应的职业后下载或查看代码</h5>
            <el-select v-model="codevalue" placeholder="请选择您的职业" style="margin-bottom:10px;">
              
            <el-option
              v-for="item in codeoptions"
              :key="item.value"
              :label="item.label"
              :value="item.value">
            </el-option>
            
            </el-select>
            <el-row>
            <el-button size="small"type="primary"icon="el-icon-view" @click="lookcode()">查看代码</el-button>
            <el-button size="small"type="success"icon="el-icon-download" style="margin:2px;"  @click="downloadcode()">点击下载</el-button>
            </el-row>
        </el-card>
    </div>    
    <div class="empty_content"></div>
    <div class="battle_content">
        <el-card shadow="always" style="width:100%;">
            <el-table
            :data="tableData"
            :default-sort = "{prop: 'teamname', order: 'descending'}"
            stripe
            style="width: 100%">
            <el-table-column
            label="#"
            min-width="10%">
                <template slot-scope="scope">{{scope.$index}}</template>
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
      <el-col :span="6"><el-checkbox :label="tableData[index-1].id" style="margin:3px;">{{tableData[index-1].teamname}}</el-checkbox></el-col>
      <el-col :span="6" v-if="index<tableData.length"><el-checkbox :label="tableData[index].id" style="margin:3px;">{{tableData[index].teamname}}</el-checkbox></el-col>
      <el-col :span="6" v-if="index+1<tableData.length"><el-checkbox :label="tableData[index+1].id" style="margin:3px;">{{tableData[index+1].teamname}}</el-checkbox></el-col>
      <el-col :span="6" v-if="index+2<tableData.length"><el-checkbox :label="tableData[index+2].id" style="margin:3px;">{{tableData[index+2].teamname}}</el-checkbox></el-col>
      </template>
      </el-row>
      <!-- <el-row>
      <el-checkbox label="复选框 B"></el-checkbox>
      </el-row>
      <el-checkbox label="复选框 C"></el-checkbox>
      <el-checkbox label="禁用" disabled></el-checkbox>
      <el-checkbox label="选中且禁用" disabled></el-checkbox> -->
     </el-checkbox-group>
     <h2>AI数量:<el-input-number v-model="AInum"  :min="0" :max="999" size="small" style="margin-left:5px;"></el-input-number></h2>
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
            checkList: [],
            AInum:0,//AI人数
            tableData: [{
            teamname:'划水萌新',
            captain:'萌新1号',
            teamid:0,
            score:20,
            }],
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
            this.tableData = res;
            for (var i = 0; i < this.tableData.length; i++) {
              for (var j = 0; j < this.tableData[i].membersID.length; j++) {
                if(id==this.tableData[i].membersID[j])
                {
                  this.teamid=this.tableData[i].teamid;
                }
              }
            }
          },error=>{
            this.$message.error("获取队伍信息失败！")
          })
    },
    methods: {
      jsReadFiles(file)
      {
          var file = files[0];
          console.log(file);
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
          }).then(res => res.blob().then(blob => { 
              var a = document.createElement('a'); 
              var url = window.URL.createObjectURL(blob);   // 获取 blob 本地文件连接 (blob 为纯二进制对象，不能够直接保存到磁盘上)
              var filename = res.headers.get('Content-Disposition'); 
              a.href = url; 
              a.download = filename; 
              a.click(); 
              window.URL.revokeObjectURL(url);
          }))
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
              a.download = filename; 
              a.click(); 
              window.URL.revokeObjectURL(url);
          }))
      },
      myUpload(content)
      {
          
          // this.$confirm('若之前已经上传过该职业的代码，上传代码会覆盖之前已有的代码，是否确定上传?',"提示",
          // {
          //   confirmButtonText: "确定",
          //   cancelButtonText: "取消",
          //   dangerouslyUseHTMLString: true,
          //   type: "warning"
          // })
          // .then(() => {
            // console.log(content);
          console.log(content.file);
          // console.log(this.fileList);
          console.log(typeof content.file);
          // console.log(this.value.toString());

          // var filepath="C:/Users/nikelong/Desktop/1.cpp";
    
          // var file = new File(filepath,"ok");
          // console.log(file);

          var fileobj=content.file;
          // var URL=;
          // var form=new FormData();
          // form.append(this.value.toString(),content.file);

          var form = new FormData();
          form.append(this.value.toString(),content.file);
          console.log(form.get('code0'));


          this.$http.post("/api/codes/teams/"+this.teamid,form,{
            emulateJSON:true,
            headers:{
              "content-type": "multipart/form-data",
               "Content-Type": "multipart/form-data",
              "x-access-token":JSON.stringify({"token":token,"id":id,"username":username,"auth":true})
            },
            }).then(function(res){
              // document.write(res.body);    
          },function(res){
              console.log(res.status);
          });
          // var settings = {
          //   "async": true,
          //   "crossDomain": true,
          //   "url": "https://teamstyle.eesast.com/api/codes/teams/"+this.teamid,
          //   "method": "POST",
          //   "headers": {
          //     "x-access-token": JSON.stringify({"token":token,"id":id,"username":username,"auth":true}),
          //     "cache-control": "no-cache",
          
          //   },
          //   "processData": false,
          //   "contentType": false,
          //   "mimeType": "multipart/form-data",
          //   "data": form
          // }

          // $.ajax(settings).done(function (response) {
          //   console.log(response);
          // });

          // fetch("/api/codes/teams/"+this.teamid,{
          //   method:'POST',
          //   headers:{
          //     "content-type": "multipart/form-data",
          //      "Content-Type": "multipart/form-data",
          //     "x-access-token":JSON.stringify({"token":token,"id":id,"username":username,"auth":true})
          //   },
          //   body:form,
          // }).then(response=>{
          //   this.fileList=[];
          //   if(response.status=="204")
          //   {
          //     this.$message.success('上传成功!');             
          //   }
          //   else if(response.status=="401")
          //   {
          //     this.$message.error('你不在这个组中!');
          //   }
          //   else if(response.status=="403")
          //   {
          //     this.$message.error('不在系统开放时间内!');
          //   }
          //   else 
          //   {
          //     this.$message.error('上传失败!');
          //   }
          // })

          // })
      },
      handleRemove(file, fileList) {
        console.log(file, fileList);
      },
      handlePreview(file) {
        console.log(file);
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
          //       // console.log(this.currentPage)  //点击第几页
        // },
        handleChecked(value) {
        let checkedCount = value.length;
        this.isIndeterminate = checkedCount > 0 && checkedCount < this.tableData.length;
      },
        start_fight()  //发起一场战斗
        {
          this.dialogTableVisible=false;//隐藏对话框
          console.log(this.checkList);//勾选的队伍
          //需要根据勾选的队伍，传送一场比赛  checkList里面记录的是选中的队伍的ID，当且仅当el-checkBox的:label绑定的是x.id;   
          //当el-checkBox 的:label绑定的是x.teamname时，checkList记录选中队伍的teamname

          //以下将传送checkList这些标号的队伍，以及AInum个AI进行对战   *********************
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