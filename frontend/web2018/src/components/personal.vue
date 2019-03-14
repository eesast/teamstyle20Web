<template>
<div id="personal">
    <div class="part1">
        <el-card shadow="always" style="text-align:left;">
            <h4>账号：{{name}}</h4>
            <h4>密码：{{password}}<el-button size="mini" type="primary" icon="el-icon-edit" @click="toalter()"></el-button></h4>
            <h4>绑定手机号：{{phone}}</h4>
            <h4>绑定邮箱号：{{mail}}</h4>
            <h4 v-if="flag0">创建的队伍：{{team}}</h4>
            <h4 v-else="flag0">加入的队伍：{{team}}</h4>
            <h4>对战比分：{{score}}</h4>
            <h4>当前排名：{{rank}}</h4>
            <el-collapse v-model="activeNames" @change="handleChange">
            <el-collapse-item title="队伍简介" name="1">
                {{intro}}
            </el-collapse-item>
            </el-collapse>
        </el-card>
    </div>    
    <div class="empty"></div>
    <div class="part2" style="vertical-align: top;">
        <el-card shadow="always"  style="text-align:left;">
        <h4>分数曲线：</h4>
        <el-collapse v-model="activeNames" @change="handleChange">
            <el-collapse-item title="对战历史" name="2">
                <el-table :data="tableData" style="width: 100%"  max-height="300" :cell-class-name="tableStyle">
            <el-table-column prop="color" min-width="1%" >
                <template slot-scope="scope">
                <div class="nothing" >&nbsp;</div>
                </template>
            </el-table-column>
            <el-table-column prop="teams" label="对战队伍" min-width="25%">
            </el-table-column>
            <el-table-column prop="result" label="对战结果" min-width="20%">
            </el-table-column>
            <el-table-column prop="download" min-width="52%">
                <template slot-scope="scope">
                <el-button  @click="perdownload">下载回放</el-button>
                </template>
            </el-table-column>
            </el-table>
            </el-collapse-item>
            </el-collapse>
        </el-card>
    </div>
    <div class="empty_content"></div>
</div>
</template>
    
<script>
var token=getCookie("token")
var current_username=getCookie("username")
var current_id=getCookie("id")
if(token==null)
{
    this.$message.error("您尚未登录，请先登录，再查看个人中心！")
    this.$router.replace({path: '/Login'})
}
export default {
    name: 'personal',
    data() {
        return {
            flag0: false,
            intro:"jjafhadhfuka",
            name: "test",
            password:"********",
            team: "one",
            phone: "12345678899",
            mail: "123456789@163.com",
            score: 100,
            rank: 5,
            tableData:[
                {
                    teams: "team1",
                    result: "victory"
                },
                {
                    teams: "team2",
                    result: "fail"
                },
                {
                    teams: "team3",
                    result: "fail"
                },
                {
                    teams: "team4",
                    result: "victory"
                },
                {
                    teams: "team2",
                    result: "fail"
                },
                {
                    teams: "team3",
                    result: "fail"
                },
                {
                    teams: "team4",
                    result: "victory"
                },
                {
                    teams: "team2",
                    result: "fail"
                },
                {
                    teams: "team3",
                    result: "fail"
                },
                {
                    teams: "team4",
                    result: "victory"
                }
            ]
        }
    },
    methods: {
        perdownload(){
            //这里时下载函数
        },
        tableRowClassName({row, rowIndex}) {
        if (row.result=== "fail") {
          return 'warning-row';
        } 
        else {
          return 'success-row';
        }
      },
      toalter()
      {
          this.$router.replace({
                path: '/alter',
        })
      },
      tableStyle({ row, column, rowIndex, columnIndex }) {
            if(columnIndex==0)
            {
                if(this.tableData[rowIndex].result=='victory')return 'statecolor'
                else return 'statecolor2'
            }
        },
    },
    created:{
        function(){
            let cur1 = document.querySelectorAll("div[class='part2']");
            let cur2 = document.querySelectorAll("div[class='part2']");
            cur2.height=cur1.height;

        }
    }
}

function getCookie(cname){
    var name = cname + "=";
    var ca = document.cookie.split(';');
    for(var i=0; i<ca.length; i++) {
        var c = ca[i].trim();
        if (c.indexOf(name)==0) { return c.substring(name.length,c.length); }
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
function setCookie(cname,cvalue){
      document.cookie = cname+"="+cvalue+";path=/";
    }

</script>

<style>
.statecolor{
    background: red;
}
.statecolor2{
    background: #909399;
}
#personal .el-table .warning-row {
    background: tomato;
  }

#personal .el-table .success-row {
    background: greenyellow;
  }
#personal .el-button{
    float: right;
}
#personal .el-card__body{
    height: 550px;
}
#personal
{
    position: relative;
    width:100%;
    min-height: 70%;
    margin-bottom:50px; 
}
#personal .el-table_1_column_3     is-leaf{
   
}
#personal .el-table_1_column_1{
    /* background-color: green; */
}
#personal .part1,#personal .part2,#personal .empty
{
    display: inline-block;
    margin-top:30px;
}
#personal .part1
{
    width:20%;
    left:5%;
}
#personal .empty
{
    width:5%;
}
#personal .part2
{
    width:70%;
    float:top;
}
</style>