# frontend
the frontend for teamstyle 20.

a new project with nothing.

zl更新：添加了nav导航栏，问题：无法实现跳转！似乎要安装Vue Router

### 2019/2/5:
* 组队界面:
```
修改inteam(Boolean)可以改变[我的队伍]的显示
当inteam==true并且iscaptain==true的时候，会显示"移出队伍按钮"
```
* 对战界面:
基本UI已经完成，与后端交互的部分尚待后端完成相应api后再做


##### 一点思考:
* 公告界面，未读公告？
不妨对每个公告标号，然后设置对应的标志(localstorage)，未读的localstorage不设置（或设置为false），已读的设置对应值(true)。然后未读的公告显示右上角的New标记~



### UI:
* 首页路由返回不显示背景bug修复
* 组队按钮、加入队伍高度修改
* 对战发起对战按钮修改(已选定的队伍id放在checkList里面)，AI暂定自动填充
* 文件界面修改
* nav修改logo大小为40x40，退出登陆设置为红色字体
