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
    <div class="part2">
        <el-card shadow="always">
            part2
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
            mail: "123456789@163.com"
        }
    },
    methods: {
         
    }
}
</script>

<style>
#personal
{
    position: relative;
    width:100%;
    min-height: 70%;
}
.part1,.part2,.empty
{
    display: inline-block;
    position: relative;
    margin-top:30px;
}
.part1
{
    width:35%;
    left:10%;
    /* min-height: 100%; */
    /* min-height:100%; */
    /* background: black; */
    /* position: relative; */
}
.empty
{
    width:14%;
}
.part2
{
    /* left:5%; */
    /* height:100%; */
    /* top:; */
    width:35%;
    /* min-height:70%; */
    /* position: relative; */
}
</style>