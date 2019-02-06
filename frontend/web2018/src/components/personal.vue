<template>
<div id="battle_contain">
    <div class="code_content">
        <el-card shadow="always" style="text-align:left;">
            part1
        </el-card>
    </div>    
    <div class="empty_content"></div>
    <div class="battle_content">
        <el-card shadow="always">
            part2
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
            
        }
    },
    methods: {
         
    }
}
</script>

<style>
#battle_contain
{
    position: relative;
    width:100%;
    min-height: 70%;
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
    width:14%;
}
.battle_content
{
    /* left:5%; */
    /* height:100%; */
    /* top:; */
    width:50%;
    /* min-height:70%; */
    /* position: relative; */
}
</style>