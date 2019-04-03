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
            <h4 v-show="showteaminfo">对战比分：{{score}}</h4>
            <h4 v-show="showteaminfo">当前排名：{{rank}}</h4>
            <el-collapse v-model="activeNames" @change="handleChange" v-show="showteaminfo">
            <el-collapse-item title="队伍简介" name="1">
                {{intro}}
            </el-collapse-item>
            </el-collapse>
        </el-card>
    </div>    
    <div class="empty"></div>
    <div class="part2" style="vertical-align: top;">
        <el-card shadow="always"  style="text-align:left;">
        <!-- <h4>分数曲线：</h4> -->
         <h5><i class="el-icon-info"></i>对战状态分为"排队中"、"对战中"和"对战结束"，请一段时间后刷新网页更新对战状态</h5>
        <el-collapse v-model="activeNames" @change="handleChange" accordion>


            <!-- 主动对战结果 -->
            <el-collapse-item title="主动对战历史" name="2">
                <el-table :data="tableDataofactive" style="width: 100%"  max-height="300" 
                empty-text="暂无对战历史"
                >
                <el-table-column prop="battle_id" label="对战编号" min-width="10%" >
                   <template slot-scope="scope">
                        <span v-if="scope.row.state==0">{{scope.row.battle_id}}</span>
                        <span v-else><i class="el-icon-loading"></i></span>
                    </template>
                </el-table-column>
                <el-table-column prop="winner" label="吃鸡队伍" min-width="20%" v-if="mobile==false">
                    <template slot-scope="scope">
                        <span v-if="scope.row.state==0">{{scope.row.winner}}</span>
                        <span v-else><i class="el-icon-loading"></i></span>
                    </template>
                </el-table-column>
                <el-table-column prop="rank" label="队伍排名" min-width="20%">
                     <template slot-scope="scope">
                        <span v-if="scope.row.state==0">{{scope.row.rank}}</span>
                        <span v-else><i class="el-icon-loading"></i></span>
                    </template>
                </el-table-column>
                <el-table-column prop="score" label="得分" min-width="20%">
                     <template slot-scope="scope">
                        <span v-if="scope.row.state==0">{{scope.row.score}}</span>
                        <span v-else><i class="el-icon-loading"></i></span>
                    </template>
                </el-table-column>
                <el-table-column prop="state" label="状态" min-width="20%">
                    <template slot-scope="scope">
                        <span v-if="scope.row.state==2"><i class="el-icon-loading"></i>对战中</span>
                        <span v-if="scope.row.state==1"><i class="el-icon-loading"></i>排队中</span>
                        <span v-if="scope.row.state==0">对战结束</span>
                    </template>
                </el-table-column>
                <el-table-column prop="download" min-width="30%">
                    <template slot-scope="scope">
                    <el-button  size="small" @click="perdownload(scope.$index,1)" :disabled="scope.row.state!=0">下载回放</el-button>
                    <el-button  size="small" @click="perdetail(scope.$index,1)" :disabled="scope.row.state!=0">查看详情</el-button>
                    </template>
                </el-table-column>
                </el-table>
            </el-collapse-item>



            <!-- 被动对战结果 -->
            
            <el-collapse-item title="被挑战历史" name="3">
                <el-table :data="tableDataofpassive" style="width: 100%"  max-height="300" 
                empty-text="暂无对战历史"
                >
            <el-table-column prop="battle_id" min-width="10%" >
                <template slot-scope="scope">
                    <span v-if="scope.row.state==0">{{scope.row.battle_id}}</span>
                    <span v-else><i class="el-icon-loading"></i></span>
                </template>
            </el-table-column>
            <el-table-column prop="winner" label="吃鸡队伍" min-width="20%" v-if="mobile==false">
                <template slot-scope="scope">
                        <span v-if="scope.row.state==0">{{scope.row.winner}}</span>
                        <span v-else><i class="el-icon-loading"></i></span>
                </template>
            </el-table-column>
            <el-table-column prop="rank" label="队伍排名" min-width="20%">
                 <template slot-scope="scope">
                        <span v-if="scope.row.state==0">{{scope.row.rank}}</span>
                        <span v-else><i class="el-icon-loading"></i></span>
                </template>
            </el-table-column>
            <el-table-column prop="score" label="得分" min-width="20%">
                 <template slot-scope="scope">
                        <span v-if="scope.row.state==0">{{scope.row.score}}</span>
                        <span v-else><i class="el-icon-loading"></i></span>
                </template>
            </el-table-column>
           <el-table-column prop="state" label="状态" min-width="20%">
                <template slot-scope="scope">
                    <span v-if="scope.row.state==2"><i class="el-icon-loading"></i>对战中</span>
                    <span v-if="scope.row.state==1"><i class="el-icon-loading"></i>排队中</span>
                    <span v-if="scope.row.state==0">对战结束</span>
                </template>
            </el-table-column>
            <el-table-column prop="download" min-width="30%">
                <template slot-scope="scope">
                <el-button  size="small" @click="perdownload(scope.$index,2)" :disabled="scope.row.state!=0">下载回放</el-button>
                <el-button  size="small" @click="perdetail(scope.$index,2)" :disabled="scope.row.state!=0">查看详情</el-button>
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
// function 
// function 
var token=getCookie("token")
var username=getCookie("username")
var id = parseInt(getCookie("id"));
if(token==null)
{
    this.$message.error("您尚未登录，请先登录，再查看个人中心！")
    this.$router.replace({path: '/Login'})
}
export default {
    name: 'personal',
    data() {
        return {
            flag0: false,      //用于标记加入或创建，如果为队长，则为true--创建
            //否则，为队员，false-加入
            intro:"暂无数据",//队伍简介  description
            name: "暂无数据",//账号名称
            password:"********",
            team: "暂无数据",//teamname
            mobile:false,
            teamid:0,
            phone: "暂无数据",//phone
            mail: "暂无数据",//mail
            score: 100,//teamscore
            rank: 5,//teamrank
            activeNames: [],
            showteaminfo:false,//是否加入队伍
            history_active:[],
            history_passive:[],
            tableDataofactive:[],
            // {
            //     teams: [1,2],
            //     teams:[],
            //     ainum: 5,
            //     state: 0,
            //     initiator_id: 2,
            //     winner: "yyr2",
            //     battle_id:2,
            //     rank: 2,
            //     score: 300
            // }],
            tableDataofpassive:[],
            // {
            //     teams: [1,2],
            //     teams:[],
            //     ainum: 5,
            //     state: 2,
            //     initiator_id: 2,
            //     winner: "yyr2",
            //     rank: 2,
            //     score: 300
            // }]
        }
    },
    methods: {
        add_active(i)
        {
            fetch('/api/battle/result?battle_id='+this.history_active[i]+'&team_id='+this.teamid,{
                method:'GET',
                headers:{
                        "Content-Type": "application/x-www-form-urlencoded",
                        "x-access-token":JSON.stringify({"token":token,"id":id,"username":username,"auth":true})
                },
                // body:JSON.stringify({
                //     battle_id:this.history_active[i],
                // })
            }).then(responsex=>{
                if(responsex.ok)
                {
                    return responsex.json();
                }
                else throw 'bad';
            }).then(resx=>{
                // this.tableDataofactive[i]=resx;
                this.$set(this.tableDataofactive,i,{ainum:resx.ainum,initiator_id:resx.initiator_id,rank:resx.rank,score:resx.score,teams:resx.teams,state:resx.state,winner:resx.winner,battle_id:resx.battle_id});
                // this.tableDataofactive[i].teams=resx.teams;
            }).catch(()=>{this.$message.error('您不在队伍中')})
        },
        add_passive(i)
        {   
            fetch('/api/battle/result?battle_id='+this.history_passive[i]+'&team_id='+this.teamid,{
                method:'GET',
                headers:{
                        "Content-Type": "application/x-www-form-urlencoded",
                        "x-access-token":JSON.stringify({"token":token,"id":id,"username":username,"auth":true})
                },
                // body:JSON.stringify({
                //     battle_id:this.history_passive[i],
                // })
            }).then(responsex=>{
                if(responsex.ok)
                {
                    return responsex.json();
                }
                else throw 'bad';
            }).then(resx=>{
                // this.tableDataofpassive[i]=resx;
                this.$set(this.tableDataofpassive,i,{ainum:resx.ainum,initiator_id:resx.initiator_id,rank:resx.rank,score:resx.score,teams:resx.teams,state:resx.state,winner:resx.winner,battle_id:resx.battle_id});
                // this.tableDataofpassive[i].teams=resx.teams;
           }).catch(()=>{this.$message.error('您不在队伍中')})

        },
        handleChange()
        {

        },
        perdetail(idx,num){
            //这里是弹出一个框显示对战结果
            console.log(idx);

            if(num==1)//active
            {
                this.$alert('对战队伍:'+this.tableDataofactive[idx].teams+'<br/>AI数量:'+this.tableDataofactive[idx].ainum+'<br/>吃鸡队伍:'+this.tableDataofactive[idx].winner+'<br/>本次比赛队伍排名:'+this.tableDataofactive[idx].rank+'<br/>本场比赛得分:'+this.tableDataofactive[idx].score, 
                '对战详情', {
                confirmButtonText: '确定',
                dangerouslyUseHTMLString: true,
                callback: action => {
                    // this.$message({
                    //   type: 'info',
                    //   message: `action: ${ action }`
                    // });
                }
                })
            }
            else if(num==2)//passive
            {
                this.$alert('对战队伍:'+this.tableDataofpassive[idx].teams+'<br/>AI数量:'+this.tableDataofpassive[idx].ainum+'<br/>吃鸡队伍:'+this.tableDataofpassive[idx].winner+'<br/>本次比赛队伍排名:'+this.tableDataofpassive[idx].rank+'<br/>本场比赛得分:'+this.tableDataofpassive[idx].score, 
                '对战详情', {
                confirmButtonText: '确定',
                dangerouslyUseHTMLString: true,
                callback: action => {
                    // this.$message({
                    //   type: 'info',
                    //   message: `action: ${ action }`
                    // });
                }
                })
            }
             
        },
        perdownload(idx,num){
            //这里是下载函数
            var battle_id=(num==1)?this.history_active[idx]:this.history_passive[idx];
            fetch('/api/playback/'+battle_id,{
                method:'GET',
                headers:{
                      "content-type": "application/octet-stream",
              "x-access-token":JSON.stringify({"token":token,"id":id,"username":username,"auth":true})
                }
            }).then(res => res.blob().then(blob => { 
              var a = document.createElement('a'); 
              var url = window.URL.createObjectURL(blob);   // 获取 blob 本地文件连接 (blob 为纯二进制对象，不能够直接保存到磁盘上)
              var filename = res.headers.get('Content-Disposition'); 
              a.href = url; 
              a.download = "对战编号:"+filename.substring(filename.length-8); 
              a.click(); 
              window.URL.revokeObjectURL(url);
          })).catch(()=>{
            this.$message.error('您没有权限!');
          })
        },
        tableRowClassName({row, rowIndex}) {
        if (row.rank!== 1) {
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
      tableStyleofactive({ row, column, rowIndex, columnIndex }) {
            if(columnIndex==0)
            {
                if(this.tableDataofactive[rowIndex].rank==1)return 'statecolor'
                else return 'statecolor2'
            }
        },
        tableStyleofpassive({ row, column, rowIndex, columnIndex }) {
            if(columnIndex==0)
            {
                if(this.tableDataofpassive[rowIndex].rank==1)return 'statecolor'
                else return 'statecolor2'
            }
        },
    },
    created: function()
    {
        if(window.screen.width<768)
        {
        this.mobile=true;
        }
        let cur1 = document.querySelectorAll("div[class='part2']");
        let cur2 = document.querySelectorAll("div[class='part2']");
        cur2.height=cur1.height;

        this.name=username;

        //剩下的重复访问user
        fetch('/api/users/'+id,
        {
            method:'GET',
            headers: {
            "Content-Type": "application/json",
            "x-access-token":JSON.stringify({"token":token,"id":id,"username":username,"auth":true})
            }
        }).then(response=>
        {
                if(response.status=="200")
                {
                    return response.json();
                }
                else if(response.status=="401")
                {
                    this.$message.error("token失效，请重新登录！");
                    if(token!=null)
                    {
                        delCookie("token")
                        delCookie("id")
                        delCookie("username")
                        token=null
                        username=null
                        id=null
                        setTimeout(() => {
                        window.location="https://teamstyle.eesast.com/login";
                        }, 100)
                    }
                }
                else
                {
                    this.$message.error("服务器暂时无法响应！");
                }
                throw 'bad';
        }).then(res=>{
            this.mail=res.email;
            this.phone=res.phone;

            //加入队伍后信息

            fetch('/api/teams/0/members/'+id,
            {
                method:'GET',
                headers: {
                "Content-Type": "application/json",
                "x-access-token":JSON.stringify({"token":token,"id":id,"username":username,"auth":true})
                }
            }).then(response=>
            {
                if(response.status=="404")
                {
                    //没有队伍
                    this.$message.error('您没有加入队伍！');
                    this.showteaminfo=false;
                    this.team="none";
                    // throw '';
                }
                else if (response.status=="200") {
                return response.json();
                } 
                else if (response.status == "401") {
                this.$message.error("token失效，请重新登录！");
                if(token!=null)
                {
                    delCookie("token")
                    delCookie("id")
                    delCookie("username")
                    token=null
                    username=null
                    id=null
                    setTimeout(() => {
                      window.location="https://teamstyle.eesast.com/login";
                    }, 100)
                }
                }
                else
                {
                    this.$message.error("服务器暂时无法响应！");
                }
                throw 'bad';
            },error=>
            {
                this.$message.error("加载失败，请稍后刷新页面重试！")
            }).then(res=>
            {
                var ans=res;//取出object

                if(ans.captainID==id)this.flag0=true;//是队长
                else this.flag0=false;
                this.showteaminfo=true;
                this.intro=ans.description;
                this.team=ans.teamname;
                this.score=ans.score;
                this.rank=ans.rank; 
                this.history_active=ans.history_active;
                this.history_passive=ans.history_passive;
                this.history_active.reverse();
                this.history_passive.reverse();
                this.teamid=ans.teamid;

                //更新active和passive
                for(var i=0;i<this.history_active.length;i++)
                {
                    // tableDataofactive
                    this.add_active(i);
                    
                }

                for(var i=0;i<this.history_passive.length;i++)
                {
                    this.add_passive(i);
                    
                }




            },error=>
            {

            })

            
            //***** */



        },error=>{})

               
        
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

@media screen and (max-width:720px) {
    #personal .part1,#personal .part2,#personal .empty
    {
        display: block;
        /* margin-top:30px; */
        top:8vh;
    }
    #personal .part1
    {
        position: relative;
        width:100%;
        left:0%;
    }
    #personal .part2
    {
        width:100%;
        position: relative;
        left:0%;
        float:top;
        margin-bottom:8vh;
    }
}
</style>