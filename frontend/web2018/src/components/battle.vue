<template>
<div id="battle_contain">
    <div class="code_content">
        <el-card shadow="always" style="text-align:left;">
            <h4>提交代码</h4>
            <el-upload
            class="upload-demo"
            action="https://jsonplaceholder.typicode.com/posts/"
            :show-file-list="false" 
            :limit="1"
            :on-success="uploadSuccess"
            :on-error="uploadError"
            
            >
            <el-button size="small" type="primary" icon="el-icon-upload">点击上传</el-button>
            </el-upload>
            <h5><i class="el-icon-info"></i>系统仅保留最后一次上传的结果</h5>
            <h5><span style="color:red"><i class="el-icon-info"></i>代码提交截止日期:3/24 24:00</span></h5>
            <br/>
            <h4>发起对战</h4>
            <h5>今日对战次数:</h5>
            <h5>剩余对战次数:</h5>
            <br/>
            <h4>查看代码</h4>
            <h5><i class="el-icon-info"></i>可以下载已上传的代码</h5>
            <el-row>
            <el-button size="small"type="primary"icon="el-icon-view">查看代码</el-button>
            <el-button size="small"type="success"icon="el-icon-download">点击下载</el-button>
            </el-row>
        </el-card>
    </div>    
    <div class="empty_content"></div>
    <div class="battle_content">
        <el-card shadow="always" style="width:100%;">
            <el-table
            :data="tableData.slice((currentPage-1)*pagesize,currentPage*pagesize)"
            :default-sort = "{prop: 'teamname', order: 'descending'}"
            stripe
            style="width: 100%">
            <el-table-column
            label="#"
            min-width="10%">
                <template slot-scope="scope">{{scope.$index+(currentPage-1)*pagesize}}</template>
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
            <el-table-column
            prop="option"
            align="center"
            label="操作"
            min-width="30%">
            <template slot-scope="scope">
                <el-button type="danger" size="mini">发起对战</el-button>
            </template>
            </el-table-column>
            </el-table>
             <el-pagination
            @current-change="handleCurrentChange"
            :current-page="currentPage"
            background
            layout="prev, pager, next"
            :total=tableData.length>
            </el-pagination>
        </el-card>
    </div>
    <div class="empty_content"></div>
</div>
</template>
    
<script>
export default {
    name: 'battle',
    data() {
        return {
            currentPage:1, //初始页
            pagesize:8,    //    每页的数据
            tableData: [{
          teamname:'划水萌新',
          captain:'萌新1号',
          score:20,
        },{
          teamname:'划水萌新2',
          captain:'萌新2号',
          score:30,
        },{
          teamname:'划水萌新2',
          captain:'萌新2号',
          score:30,
        },{
          teamname:'划水萌新2',
          captain:'萌新2号',
          score:30,
        },{
          teamname:'划水萌新2',
          captain:'萌新2号',
          score:30,
        },{
          teamname:'划水萌新2',
          captain:'萌新2号',
          score:30,
        },{
          teamname:'划水萌新2',
          captain:'萌新2号',
          score:30,
        },{
          teamname:'划水萌新2',
          captain:'萌新2号',
          score:30,
        },{
          teamname:'划水萌新2',
          captain:'萌新2号',
          score:30,
        },{
          teamname:'划水萌新2',
          captain:'萌新2号',
          score:30,
        },{
          teamname:'划水萌新2',
          captain:'萌新2号',
          score:30,
        },{
          teamname:'划水萌新2',
          captain:'萌新2号',
          score:30,
        },{
          teamname:'划水萌新2',
          captain:'萌新2号',
          score:30,
        },{
          teamname:'划水萌新2',
          captain:'萌新2号',
          score:30,
        },{
          teamname:'划水萌新2',
          captain:'萌新2号',
          score:30,
        },{
          teamname:'划水萌新2',
          captain:'萌新2号',
          score:30,
        },{
          teamname:'划水萌新2',
          captain:'萌新2号',
          score:30,
        },{
          teamname:'划水萌新2',
          captain:'萌新2号',
          score:30,
        },{
          teamname:'划水萌新2',
          captain:'萌新2号',
          score:30,
        }]
        }
    },
    methods: {
        uploadSuccess(res){
            if(res.status==1){
              this.$message({
                message: '恭喜你，上传成功',
                type: 'success'
              });
            }
            else if(res.status==0){
              this.$message({
                message: res.msg,
                type: 'warning'
              });
            }
            else{
              this.$message.error('上传失败，请重新上传');
            }
          },
          uploadError(){
            this.$refs.upload.clearFiles();
            this.$message.error('上传失败，请重新上传');
          },
          handleCurrentChange: function(currentPage){
                this.currentPage = currentPage;
                // console.log(this.currentPage)  //点击第几页
        },
        
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
    position: relative;
    margin-top:30px;
}
.code_content
{
    width:18%;
    /* min-height: 100%; */
    /* min-height:100%; */
    /* background: black; */
    /* position: relative; */
}
.empty_content
{
    width:9%;
}
.battle_content
{
    /* left:5%; */
    /* height:100%; */
    /* top:; */
    width:60%;
    /* min-height:70%; */
    /* position: relative; */
}
</style>