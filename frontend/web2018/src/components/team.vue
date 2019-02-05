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
                        <td>{{detailData.teamname}}</td>
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
                        <td><el-input type="textarea" :rows="6" resize="none"  v-model="detailData.description" autocomplete="off" readonly></el-input></td>
                    </tr>      
                    <tr class='table_th1'>
                        <th ><img src="../../static/img/team.svg"  class="svg"></img>队伍成员</th>
                        <td>
                            <table id="members_table">
                                <tr v-for="(x,index) in detailData.members" :class="'table_th'+(index%2+1)">
                                    <td valign="middle">{{detailData.members[index]}}</td>
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

        </el-tab-pane>
        </el-tabs>
    </div>
</template>

<script>
export default {
    name: "team",
    data(){
        return {
            inteam: true, //是否在队伍中
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
            detailData:
            {
                teamname:"划水大法好",
                captain:"萌新0号",
                invitecode:"1z2x3c4v",
                description:"我们最爱划水",
                members:['好人','萌新','蒟蒻']
            }
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
        }
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
    height:80%;
}
#team_card
{
    position: relative;
    width:60%;
    left:20%;
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
    height:30px;
    position: relative;
}
#members_table .table_th1
{
    background-color:white;
    padding-top:10px;
    height:30px;
    position: relative;
}
#copyinvitecode
{
    position: relative;
    left:50px;
}
</style>