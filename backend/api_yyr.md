# 对战

**关于对战状态**：用整数表示，0表示对战结束，1表示正在排队中，2表示正在对战中

## 文件存储结构

* media/Codes/：存放玩家代码，以``队伍编号_代码编号.cpp``命名。（代码编号为0\~3）
* media/Codes/output/：存放编译后的.so文件，以``队伍编号_代码编号.cpp``命名
* media/data/\<对战编号\>/：存放对战信息，对战结束后，里面将存有参与比赛的.so文件，游戏方面生成的.json文件，以及压缩后的.pb.7z回放文件

## 发起对战

路径：api/battle/add

传入接口：暂时使用GET，传入JSON格式的字符串数组teams表示参赛队伍，整数AInum表示AI数目，字符串initiator\_name表示发起对战的队伍名称。

说明：支持排队等待机制。

## 查询对战信息

路径：api/battle/result

传入接口：暂时使用GET，传入battle\_id表示要查询信息的battle编号。

返回信息：返回一个JSON，类似如下

```
{
    "teams": "[\"yyr1\", \"yyr2\"]",
    "ainum": 5,
    "state": 0,
    "initiator_name": "yyr1",
    "winner": "yyr2",
    "rank": 2,
    "score": 300
}
```

分别对应参赛队伍、AI数目、运行状态、对战发起队伍、吃鸡队伍、发起队伍的排名和分数。

## 编译玩家代码

路径：api/battle/compile  （暂时放在battle下）

传入接口：暂时使用GET，传入team\_id表示队伍编号，ind表示代码编号(0\~3)。

返回信息：返回一个JSON，"status"对应的值为"Compile successfully"或者"Compile Error"，"information"对应的值为编译具体信息

说明：玩家代码应存放在media/Codes/下，命名为``队伍编号_代码编号.cpp``，编译成功后将在media/Codes/output/下产生``队伍编号_代码编号.so``
