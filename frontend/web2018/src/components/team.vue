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
                        <td>{{detailData.teamname}}
                            <el-button v-if="iscaptain==true"type="danger" size="small" style="position:relative;float:right;" @click="clearTeam()">解散队伍</el-button>
                            <el-button v-else type="danger" size="small" style="position:relative;left:50%;" @click="exitTeam()">退出队伍</el-button>
                        </td>
                    </tr>
                    <tr class='table_th2'>
                        <th ><img src="../../static/img/user.svg" class="svg"></img>队长</th>
                        <td>{{detailData.captain}}</td>
                    </tr>
                    <tr class='table_th1'>
                        <th ><img src="../../static/img/id card.svg" class="svg"></img>邀请码</th>
                        <td>{{detailData.invitecode}}<el-button v-clipboard:copy="detailData.invitecode" v-clipboard:success="onCopy" v-clipboard:error="onError" id="copyinvitecode"type="primary">复制</el-button></td>
                    </tr>
                    <tr class='table_th2'>
                        <th ><img src="../../static/img/edit-square.svg" class="svg"></img>队伍简介</th>
                        <td><el-input type="textarea" :rows="6" resize="none"  v-model="detailData.description" autocomplete="off" readonly></el-input><el-button size="mini" type="primary" icon="el-icon-edit" style="float:right;top:5px;position:relative;" @click="edit_description()">修改简介</el-button></td>
                    </tr>      
                    <tr class='table_th1'>
                        <th ><img src="../../static/img/team.svg"  class="svg"></img>队伍成员</th>
                        <td>
                            <table id="members_table">
                                <tr v-for="(x,index) in detailData.members" :class="'table_th'+(index%2+1)">
                                    <td valign="middle">{{detailData.members[index]}}
                                        <el-button v-if="iscaptain==true" size="small" type="danger" class="dropout" @click="dropOut(index)">移出队伍</el-button>
                                    </td>
                                    <!-- <td v-if="iscaptain==true">踢出队伍</td> -->
                                </tr>
                                <!-- <tr class='table_th1'><td valign="middle">{{detailData.members[0]}}</td></tr>
                                <tr class='table_th2'><td valign="middle">2</td></tr>
                                <tr class='table_th1'><td valign="middle">3</td></tr> -->
                            </table>
                        </td>
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
            label="队伍序号"
            width="80">
                <template slot-scope="scope">
                    {{scope.$index+(currentPage-1)*pagesize}}
                </template>
            </el-table-column>
            <el-table-column
            prop="teamname"
            label="队伍名称"
            width="100">
            </el-table-column>
            <el-table-column
            prop="captain"
            label="队长"
            width="100">
            </el-table-column>
            <el-table-column
            prop="description"
            label="队伍简介"
            width="460">
            </el-table-column>
            <el-table-column
            prop="members"
            width="150"
            label="队伍成员">
                <template slot-scope="scope">
                    <!-- <table style="width:100%;background:#f5f7fa;"frame=void> -->
                        <p v-for="(x,index) in scope.row.members">
                            {{x}}
                        </p>
                    <!-- </table> -->
                </template>
            </el-table-column>
            <el-table-column
            label="操作">
                <template slot-scope="scope">
                    <el-button type="primary" @click="joinTeam(scope.$index+(currentPage-1)*pagesize)">加入队伍</el-button>
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
        </el-tab-pane>
        </el-tabs>
    </div>
</template>

<script>
export default {
    name: "team",
    data(){
        return {
            inteam: true, //是否在队伍中  影响“我的队伍”显示界面
            iscaptain: true,//该成员是否为队长  影响“我的队伍”是否出现踢人选项
            
            dialogFormVisible: false,//创建队伍对话框
            form: {
            teamname: '',
            description: '',
            // date1: '',
            // date2: '',
            delivery: false,
            // type: [],
            // resource: '',
            // desc: ''
            },
            rules:{
                teamname:[{ required: true, message: '请输入队伍名称', trigger: 'blur' },
                { min: 1, max: 10, message: '长度在 1 到 10 个字', trigger: 'blur' }],
                description:[{ required: true, message: '请输入队伍简介', trigger: 'blur' },
                { min: 1, max: 50, message: '长度在 1 到 50 个字', trigger: 'blur' }],
            },
            formLabelWidth: '100px',//对话框宽度
            detailData:   //若当前用户已有队伍，则其队伍信息
            {
                teamname:"划水大法好",
                captain:"萌新0号",
                invitecode:"1z2x3c4v",
                description:"我们最爱划水",
                members:['好人','萌新']
            },
            currentPage:1, //初始页
            pagesize:10,    //    每页的数据
            //加入队伍中所有队伍信息
            // tableData: [{ 
                //id:0,
            //     teamname:"划水大法好",
            //     captain:"萌新0号",
            //     description:"我们最爱划水",
            //     members:['好人','萌新']
            // }, ]
            tableData: [{ 
                id:0,
                teamname:"划水大法好",
                captain:"萌新0号",
                description:"我们最爱划水",
                members:['好人','萌新']
            }, {
                id:0,
                teamname:"划水大法好",
                captain:"萌新0号",
                description:"我们最爱划水",
                members:['好人','萌新','蒟蒻']
            }, {   
                id:0,
                teamname:"划水大法好",
                captain:"萌新0号",
                description:"我们最爱划水",
                members:['好人','萌新','蒟蒻']
            }, {
                teamname:"划水大法好",
                captain:"萌新0号",
                description:"我们最爱划水",
                members:['好人','萌新','蒟蒻']
            },{
                teamname:"划水大法好",
                captain:"萌新0号",
                description:"我们最爱划水",
                members:['好人','萌新','蒟蒻']
            },{
                teamname:"划水大法好",
                captain:"萌新0号",
                description:"我们最爱划水",
                members:['好人','萌新','蒟蒻']
            },{
                teamname:"划水大法好",
                captain:"萌新0号",
                description:"我们最爱划水",
                members:['好人','萌新','蒟蒻']
            },{
                teamname:"划水大法好",
                captain:"萌新0号",
                description:"我们最爱划水",
                members:['好人','萌新','蒟蒻']
            },{
                teamname:"划水大法好",
                captain:"萌新0号",
                description:"我们最爱划水",
                members:['好人','萌新','蒟蒻']
            },{
                teamname:"划水大法好",
                captain:"萌新0号",
                description:"我们最爱划水",
                members:['好人','萌新','蒟蒻']
            },{
                teamname:"划水大法好",
                captain:"萌新0号",
                description:"我们最爱划水",
                members:['好人','萌新','蒟蒻']
            },{
                teamname:"划水大法好",
                captain:"萌新0号",
                description:"我们最爱划水",
                members:['好人','萌新','蒟蒻']
            },{
                teamname:"划水大法好",
                captain:"萌新0号",
                description:"我们最爱划水",
                members:['好人','萌新','蒟蒻']
            },{
                teamname:"划水大法好",
                captain:"萌新0号",
                description:"我们最爱划水",
                members:['好人','萌新','蒟蒻']
            },{
                teamname:"划水大法好",
                captain:"萌新0号",
                description:"我们最爱划水",
                members:['好人','萌新','蒟蒻']
            },{
                teamname:"划水大法好",
                captain:"萌新0号",
                description:"我们最爱划水",
                members:['好人','萌新','蒟蒻']
            },{
                teamname:"划水大法好",
                captain:"萌新0号",
                description:"我们最爱划水",
                members:['好人','萌新','蒟蒻']
            },{
                teamname:"划水大法好",
                captain:"萌新0号",
                description:"我们最爱划水",
                members:['好人','萌新','蒟蒻']
            },{
                teamname:"划水大法好",
                captain:"萌新0号",
                description:"我们最爱划水",
                members:['好人','萌新','蒟蒻']
            }]
        }
    },
    methods: {
        createDialog()
        {
            this.dialogFormVisible = true;
            this.form.teamname='';
            this.form.description='';
        },
        createTeam()
        {
            // console.log(this.form['teamname']);
            // console.log(this.form['description']);
            if(this.form['teamname'].length<=10&&this.form['teamname'].length>0)//
            {
                if(this.form['description'].length<=50&&this.form['description'].length>0)
                {
                    //***************上传数据 */
                    this.$message.success('创建队伍成功');//写在回调函数中
                    this.dialogFormVisible = false;
                }
                else if(this.form['description'].length==0)
                {
                    this.$message.error('请输入队伍简介');
                }
                else
                {
                    this.$message.error('队伍简介过长');
                }
            }
            else if(this.form['teamname'].length==0)//显示‘请输入队伍名称’
            {
                this.$message.error('请输入队伍名称');
            }
            else//显示‘队伍名称过长’
            {
                this.$message.error('队伍名称过长');
            }
        },
        onCopy()
        {
            
            this.$message.success('复制成功');
        },
        onError()
        {
            this.$message.error('复制失败');
        },
        dropOut(index)//踢出id为index的队员
        {
             this.$confirm('是否确定要将队友&nbsp;&nbsp;&nbsp;<span style="color:red">'+this.detailData.members[index]+'</span>&nbsp;&nbsp;&nbsp;请出队伍?', '提示', {
                confirmButtonText: '确定',
                cancelButtonText: '取消',
                dangerouslyUseHTMLString: true ,
                type: 'warning'
                }).then(() => {
                    //****************发送命令将队员从队伍中删除 */
                    // fetch()
                this.$message({
                    type: 'success',
                    message: '操作成功!'
                });
                }).catch(() => {
                this.$message({
                    type: 'info',
                    message: '已取消删除'
                });          
                });
        },
        joinTeam(index)//加入第几个队伍
        {
            // console.log(index);
            //this.tableData[index].id//为加入队伍的标号
            var teamid=this.tableData[index].id;//获取队伍标号

            this.$prompt('请输入邀请码', '提示', {
            confirmButtonText: '确定',
            cancelButtonText: '取消',
            // inputPattern: /[\w!#$%&'*+/=?^_`{|}~-]+(?:\.[\w!#$%&'*+/=?^_`{|}~-]+)*@(?:[\w](?:[\w-]*[\w])?\.)+[\w](?:[\w-]*[\w])?/,
            // inputErrorMessage: '邮箱格式不正确'
            }).then(({ value }) => {
            //传入value作为teamid队伍的邀请码
            // fetch('',()=>
            // {

            // }).then(response=>({
            //     if(ok)
            //     {
            //           this.$message({
            //                 type: 'success',
            //                 message: '加入队伍成功'
            //             });
            //     }
            //     else if(no)
            //     {
            //         this.$message({
            //                 type: 'error',
            //                 message: '加入队伍失败'
            //         });
            //     }
            // }))    


            }).catch(() => {
            // this.$message({
            //     type: 'info',
            //     message: '取消加入队伍'
            // });       
            });
        },
        clearTeam()//队长解散队伍
        {

        },
        exitTeam()//队员退出队伍
        {

        },
        edit_description()//修改队伍简介
        {

        },
        handleCurrentChange: function(currentPage){
                this.currentPage = currentPage;
                // console.log(this.currentPage)  //点击第几页
        },
    }
}
</script>

<style>
.svg
{
    position: relative;
    padding-right:12px; 
    top:3px;
    width:20px;
    height:20px;
    /* padding: 12px; */
}
#team_contain
{
    width:100%;
    min-height:70%;
    margin-bottom:50px; 
}
#team_card
{
    position: relative;
    width:70%;
    left:15%;
    margin-top:2%;
}
#create_team
{
    width:100%;
    position: relative;
    /* padding:20px; */
}
.create_team
{
    position: relative;
    margin-top:20px;
}
#create_team_dialog
{
    width:100%;
    /* position: relative; */
}
#detail_table
{
    padding: 12px 0;
    min-width: 0;
    width:100%;
    -webkit-box-sizing: border-box;
    box-sizing: border-box;
    text-overflow: ellipsis;
    vertical-align: middle;
    position: relative;
    text-align: left;
    color: #909399;
    border-collapse: collapse;
}
#detail_table tr
{
    /* border:1px solid red; */
    width:100%;
    padding:12px;
    border-bottom: 1px solid #f5f7fa;
}
#detail_table tr th
{
    padding:12px;
}
#detail_table tr th i
{
    padding-right:12px;
}
#detail_table tr td
{
    position: relative;
    padding:12px;
    color:black;
    /* left:200px; */
}
.table_th2
{
    background-color: #f5f7fa;
}
.table_th1
{
    background-color:white;
}
#members_table
{
    width:100%;
}
#members_table .table_th2
{
    background-color: #f5f7fa;
    padding-top:10px;
    height:50px;
    position: relative;
}
#members_table .table_th1
{
    background-color:white;
    padding-top:10px;
    height:50px;
    position: relative;
}
#copyinvitecode
{
    position: relative;
    /* left:50px; */
    float:right;
}
.dropout
{
    position: relative;
    /* left:50%; */
    float:right;
}
</style>