<template>
    <div id="file_contain">
    <el-col :span="8" id="file_card">
        <el-card shadow="always">
            <el-table
            :data="tableData"
            stripe
            style="width: 100%;font-size:2.2vh;min-height:60vh;">
            <el-table-column min-width="1%"></el-table-column>
            <el-table-column
            prop="filename"
            label="文件名"
            align="left"
            id="tl1"
            min-width="55%">
            </el-table-column>
            <el-table-column
            align="center"
            prop="download"
            min-width="40%"
            id="tl2"
            label="下载">
                <template slot-scope="scope">
                <a :href="scope.row.download" :download="scope.row.filename"><el-button size="small">Download</el-button></a>
                </template>
            </el-table-column>
            <!-- <el-table-column
            id="tl3"
            min-width="25%"
            align="right">
            <template slot="header" slot-scope="scope">
                 <el-tooltip class="item" effect="dark" content="下载所有文件并打包成zip" placement="right-start">
                        <el-button  icon="el-icon-download" size="mini" type="primary" @click="handleBatchDownload()"><span id="downloadall">Download All</span></el-button>
                </el-tooltip>
            </template>
            </el-table-column> -->
            </el-table>
             <el-tooltip class="item" effect="dark" content="下载所有文件并打包成zip" placement="right-start">
                        <el-button  icon="el-icon-download" size="mini" type="primary" @click="handleBatchDownload()" style="margin-top:15px;">Download All</el-button>
            </el-tooltip>
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
                filename:'ts20-0.1alpha.zip',
                download:'../../static/files/ts20-0.1alpha.zip'
            },{
                filename:'README.MD',
                download:'../../static/files/README.MD'
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
    },

    created:function(){
            var FETCH_URL="/api/file/list"
            fetch(FETCH_URL, {
            method: "GET",
            headers: {
              "content-type": "application/json"
            }
          })
            .then(response => {
              console.log(response.status);
              if (response.ok) {
                return response.json();
              } else {
                  this.$message.error("获取文件列表失败!")
              }
            })
            .then(res => {
              var file_from_backend=res
              for (var i in file_from_backend)
              {
                  var single_file=new Object()
                  single_file["filename"]=file_from_backend[i]["title"]
                  single_file["id"]=file_from_backend[i]["id"]
                  single_file["last_update_date"]=file_from_backend[i]["last_update_date"]
                  var FETCH_URL1="/api/file/download/"+single_file["id"]
                  fetch(FETCH_URL1, {
                    method: "GET",
                    headers: {
                        "content-type": "application/json"
                    },
                  })
                  .then(response => {
                    console.log(response.status);
                    if (response.ok) {
                        return response.json();
                    } else {
                        this.$message.error("获取文件"+ single_file["filename"]+"失败")
                    }
                  })
                  .then(res => {
                    pass
                });
                var file_display= new Object()
                file_display["filename"]=single_file["filename"]
                //tableData.push(file_display)
              }
            });
        }
    }
</script>

<style>
#file_contain
{
    width:100%;
    min-height:90%;
}
#file_card
{
    margin-top:20px;
    width:50%;
    position: relative;
    left:25%;
}
#downloadall
{
    /* background-color: rgb(158, 209, 192); */
    /* font-size:0.8em; */
}



 @media screen and (max-width: 720px)
 { 
     #file_contain
    {
        width:100%;
        position: relative;
        top:8vh;
        min-height:90vh;
        font-size:2vh;
    }
     #file_card
    {
        margin-top:20px;
        width:90%;
        position: relative;
        left:5%;
        font-size:2vh;
    }
 }
  /* @media screen and (max-width: 992px)
 { 
     #file_card
    {
        margin-top:20px;
        width:80%;
        position: relative;
        left:10%;
    }
 } */
</style>
