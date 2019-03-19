<template>
<div id="announcement_contain">
    <el-breadcrumb separator-class="el-icon-d-arrow-right" id="announcement-breadcrumb">
    <el-breadcrumb-item :to="{ path: '/empty' }" >公告</el-breadcrumb-item>
    <el-breadcrumb-item v-if="index!=0">公告详情</el-breadcrumb-item>
    </el-breadcrumb>    

    <el-col :span="8" id="announcement_card" >
        <el-card shadow="always">
        
        <el-table
        empty-text="暂无公告"
        v-if="detail==0"
        :data="tableData"
        stripe
        style="width: 100%;font-size:2vh;height:60vh;">
            <el-table-column
            prop="title"
            label="标题"
            min-width="60%">
                <template slot-scope="scope">
                <el-badge v-if="scope.$index==-1" value="new" class="announcement-item">
                        <!-- <div slot="reference" class="name-wrapper"> -->
                        <div  @click="announcement_detail(scope.$index)">{{ scope.row.title }}</div>
                        <!-- </div> -->
                </el-badge>    
                 <div v-else  @click="announcement_detail(scope.$index)">{{ scope.row.title }}</div>
                </template>
            </el-table-column>
            
            <!-- <el-table-column
            prop="pub_date"
            label="发布时间"
            align="right"
            min-width="25%">
                <template slot-scope="scope">
                <i class="el-icon-time"></i>
                <span style="margin-left: 10px;">{{ scope.row.pub_date }}</span>
                </template>
            </el-table-column> -->
            
            <el-table-column
            prop="last_update_date"
            label="最后更新时间"
            min-width="40%"
             align="center"
            >
                <template slot-scope="scope">
                <i class="el-icon-time"></i>
                <span style="margin-left: 10px">{{ scope.row.last_update_date }}</span>
                </template>
            </el-table-column>
            <el-table-column min-width="1%"></el-table-column>
        </el-table>


        <!-- 显示具体内容的表格 -->
        <table  id="detail_table" v-else>
            <tr class='table_th1'>
                <th ><i class="el-icon-info"></i>标题</th>
                <td>{{detailData.title}}</td>
            </tr>
            <tr class='table_th2'>
                <th ><i class="el-icon-time"></i>发布时间</th>
                <td>{{detailData.pub_date}}</td>
            </tr>
            <tr class='table_th1'>
                <th ><i class="el-icon-time"></i>最后更新时间</th>
                <td>{{detailData.last_update_date}}</td>
            </tr>
            <tr class='table_th2'>
                <th ><i class="el-icon-document"></i>正文</th>
                <td>{{detailData.content}}</td>
            </tr>        
        </table>    
        </el-card>
    </el-col>
    <div id="back_div" v-if="detail!=0">
    <el-button  id="back_btn" @click="announcement_init()">返回</el-button>
    </div>
</div>
</template>
<script>
export default {
    name: "announcement",
    data(){
        return {
            index: 0,
            detail:0,
            tableData:[],
            // [{
            //     "title":"这是一个公告",
            //     "pub_date":"Dec. 1,2018,8:38 p.m.",
            //     "last_update_date":"Dec. 1,2018,8:38 p.m."
            // }, {
            //     "title":"这也是一个公告",
            //     "pub_date":"Dec. 1,2018,8:38 p.m.",
            //     "last_update_date":"Dec. 1,2018,8:38 p.m."
            // },],
            detailData:
            {
                "title":"这是一个公告",
                "content":"这只是一个公告，还没有什么内容",
                "pub_date":"Dec. 1,2018,8:38 p.m.",
                "last_update_date":"Dec. 1,2018,8:38 p.m."
            }
        };
    },
    methods: {
        announcement_init()
        {
            this.index=0;
            this.detail=0;
        },
        announcement_detail(idx)//展示标号为idx的公告的细节
        {
            // console.log(idx);
            // this.$set(this.index,1);
            this.index=1;//面包屑上显示公告详情
            this.detail=1;
            //*************api获取当前新闻内容并修改给detailData
            fetch("/api/announce/view/"+idx,
            {
                method:'GET',
                headers:
                {
                    'Content-Type':'application/json',
                },
            }).then(response=>
            {
                console.log(response.status)
                if(response.ok)
                {
                    return response.json();
                }
                else
                {
                    this.$message.error("获取公告失败");
                }
            }).then(res=>
                {
                    if (res==undefined) return
                    res=JSON.stringify(res)
                    this.detailData["title"]=res["title"]
                    this.detailData["pub_date"]=res["pub_date"]
                    this.detailData["last_update_date"]=res["last_update_date"]
                    this.detailData["content"]=res["content"]
                })
        }

    },
    created:function(){
    fetch("/api/announce/list",
      {
        method:'GET',
        headers:
        {
            'Content-Type':'application/json',
        },
      }).then(response=>
      {
        console.log(response)
        if(response.ok)
        {
          return response.json();
        }
        else
        {
          this.$message.error("获取公告失败");
        }
      }).then(res=>
      {
        if (res==undefined) return
        res=JSON.stringify(res)
        for (var i = 0; i < response_data.length; i++) 
        {
            var ann= new Object()
            ann["title"]=res[i]["title"]
            ann["pub_date"]=res[i]["pub_date"]
            ann["last_update_date"]=res[i]["last_update_date"]
            this.tableData.push(ann)
        }
        console.log(this.tableData)
    })
    }
}

</script>
<style>
#announcement_contain
{
    margin-top:20px;
    /* margin-bottom:20px; */
    width:100%;
    min-height:70%;
    /* height:70%; */
    margin-bottom: 50px;
}
#announcement_card
{
    position: relative;
    margin-top:20px;
    width:70%;
    left:15%;
}
#announcement-breadcrumb
{
    height:5%;
    font-size:20px;
    position: relative;
    left:15%;
}
.announcement-item {
  margin-top: 10px;
  margin-right: 40px;
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
#back_div
{
    position: relative;
    width:70%;
    left:15%;
    /* padding: 15px; */
}
#back_btn
{
    position: relative;
    top:20px;
}
@media screen and (max-width:720px) {
    #announcement_card
    {
        width:100%;
        left:0%;
        top:7vh;
        min-height:60vh;
        /* height:60vh; */
    }
    #announcement_contain
    {
        height:80vh;
    }
}
</style>

