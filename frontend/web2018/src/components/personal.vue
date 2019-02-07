<template>
<div id="personal">
    <div class="part1">
        <el-card shadow="always" style="text-align:left;">
            <h4>账号：{{name}}</h4>
            <h4>密码：{{password}}<el-button size="mini" type="primary" icon="el-icon-edit" index="/alter"></el-button></h4>
            <h4>绑定手机号：{{phone}}<el-button size="mini" type="primary" icon="el-icon-edit" index="/alter"></el-button></h4>
            <h4>绑定邮箱号：{{mail}}<el-button size="mini" type="primary" icon="el-icon-edit" index="/alter"></el-button></h4>
            <h4 v-if="flag0">创建的队伍：{{team}}</h4>
            <h4 v-else="flag0">加入的队伍：{{team}}</h4>
        </el-card>
    </div>    
    <div class="empty"></div>
    <div class="part2" style="vertical-align: top;">
        <el-card shadow="always"  style="text-align:left;">
        <h4>对战比分：{{score}}</h4>
        <h4>当前排名：{{rank}}</h4>
        <el-table :data="tableData" style="width: 100%" :row-class-name="tableRowClassName" max-height="400">
            <el-table-column prop="teams" label="对战队伍">
            </el-table-column>
            <el-table-column prop="result" label="对战结果">
            </el-table-column>
        </el-table>
        </el-card>
    </div>
    <div class="empty_content"></div>
</div>
</template>
    
<script>
export default {
    name: 'personal',
    data() {
        return {
            flag0: true,
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
        tableRowClassName({row, rowIndex}) {
        if (row.result=== "fail") {
          return 'warning-row';
        } 
        else {
          return 'success-row';
        }
      }
    }
}
</script>

<style>
.el-table .warning-row {
    background: tomato;
  }

  .el-table .success-row {
    background: greenyellow;
  }
.el-button{
    float: right;
}
#personal
{
    position: relative;
    width:100%;
    min-height: 70%;
}
.part1,.part2,.empty
{
    display: inline-block;
    margin-top:30px;
}
.part1
{
    width:35%;
    left:10%;
}
.empty
{
    width:8%;
}
.part2
{
    width:35%;
    float:top;
}
</style>