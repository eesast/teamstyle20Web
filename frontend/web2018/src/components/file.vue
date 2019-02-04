<template>
    <div id="file_contain">
    <el-col :span="8" id="file_card">
        <el-card shadow="always">
            <el-table
            :data="tableData"
            stripe
            style="width: 100%">
            <el-table-column
            prop="filename"
            label="文件名"
            width="650">
            </el-table-column>
            <el-table-column
            prop="download"
            width="50"
            label="下载">
                <template slot-scope="scope">
                <a :href="scope.row.download" :download="scope.row.filename"><i class="el-icon-download"></i></a>
                </template>
            </el-table-column>
            <el-table-column
            align="right">
            <template slot="header" slot-scope="scope">
                <el-button>Download All</el-button>
            </template>
            </el-table-column>
            </el-table>

        </el-card>
    </el-col>
    </div>
</template>

<script>
export default {
    name: "file",
    data(){
        return {
            tableData:
            [{
                filename:'大赛文件.pdf',
                download:'/大赛文件.pdf'
            },{
                filename:'大赛文件2.pdf',
                download:'/大赛文件2.pdf'
            },{
                filename:'大赛文件.pdf',
                download:'/大赛文件.pdf'
            },{
                filename:'大赛文件2.pdf',
                download:'/大赛文件2.pdf'
            }]
        }
    },
    methods: {

    }
}
</script>

<style>
#file_contain
{
    width:100%;
    height:80%;
}
#file_card
{
    margin-top:20px;
    width:60%;
    position: relative;
    left:20%;
}
</style>
