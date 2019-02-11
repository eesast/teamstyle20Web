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
            min-width="60%">
            </el-table-column>
            <el-table-column
            prop="download"
            min-width="10%"
            label="下载">
                <template slot-scope="scope">
                <a :href="scope.row.download" :download="scope.row.filename"><i class="el-icon-download"></i></a>
                </template>
            </el-table-column>
            <el-table-column
            min-width="30%"
            align="right">
            <template slot="header" slot-scope="scope">
                 <el-tooltip class="item" effect="dark" content="下载所有文件并打包成zip" placement="right-start">
                        <el-button size="mini" type="primary" id="downloadall"@click="handleBatchDownload()">Download All</el-button>
                </el-tooltip>
            </template>
            </el-table-column>
            </el-table>

        </el-card>
    </el-col>
    </div>
</template>

<script>
import axios from 'axios'
import JSZip from 'jszip'
import FileSaver from 'file-saver'

const getFile = url => {
    return new Promise((resolve, reject) => {
        axios({
            method:'get',
            url,
            responseType: 'arraybuffer'
        }).then(data => {
            resolve(data.data)
        }).catch(error => {
            reject(error.toString())
        })
    })
}

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
        handleBatchDownload() {
            //const data = ['/static/docker.svg', '/static/vue.png'] // 需要下载打包的路径, 可以是本地相对路径, 也可以是跨域的全路径
            var data=new Array();
            for (var i=0;i<this.tableData.length;i++)
            {
                console.log(this.tableData[i]);
                data.push('/static/files/'+this.tableData[i].download);
            }
            const zip = new JSZip()
            const cache = {}
            const promises = []
            data.forEach(item => {
                const promise = getFile(item).then(data => { // 下载文件, 并存成ArrayBuffer对象
                    const arr_name = item.split("/")
                    const file_name = arr_name[arr_name.length - 1] // 获取文件名
                    zip.file(file_name, data, { binary: true }) // 逐个添加文件
                    cache[file_name] = data
                })
                promises.push(promise)
            })

            Promise.all(promises).then(() => {
                zip.generateAsync({type:"blob"}).then(content => { // 生成二进制流
                    FileSaver.saveAs(content, "teamstyle20.zip") // 利用file-saver保存文件
                })
            })
        },
    }
}
</script>

<style>
#file_contain
{
    width:100%;
    min-height:70%;
}
#file_card
{
    margin-top:20px;
    width:60%;
    position: relative;
    left:20%;
}
#downloadall
{
    /* background-color: rgb(158, 209, 192); */
}
</style>
