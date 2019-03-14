# API文档
### · 验证部分(auth)
### · 用户部分(user)
### · 队伍系统(team)
### · 代码部分(code)
### · 公告部分(announcement)
### · 文件部分(file) 

### · 全域设定读取(globalsettings) 

##### 注:凡涉及到因token失效而返回401的，建议同时返回JSON文本:
```
401:Invalid or expired token.
```





## 验证部分
| route | http verb | content |
| ----- | --------- | ------- |
| /api/auth | POST | 使用用户名和密码登录，换取x-access-token(以及用户id) |
### 细则
* header

    | key | value |
    |-------|------|
    | content-type |application/json|
* body 
```
    {
        "username":"username",
        "password":"password"
    }
```
* response
验证成功，状态码为200,返回JSON格式的文本，例如:
```
    {
        "auth":true,
        "token":"token",
        "id":0,
        "username":"username"
    }
```
若用户不存在，则状态码为 404，将返回文本：
      404 Not Found: User does not exist.
若用户名或密码字段为空，则状态码为 422，将返回文本：
      422 Unprocessable Entity: Missing essential post data.
若用户名或密码错误，则状态码为 401，将返回 JSON 格式的文本：

```
      {
        "auth": false,
        "token": null
      }
```




## 用户部分
| route | http verb | content |
| ----- | --------- | ------- |
| /api/users | GET |获取所有用户信息|
| /api/users | POST |新增一个用户| /api/users/:id | GET | 获取相应id用户的信息 |
| /api/users/:id | PUT | 更新相应id用户信息 |
| /api/users/:id | DELETE | 删除相应id用户 |

* /api/users    GET
    * header
    | key | value |
    |-------|------|
    | x-access-token |用户token|
    * response 
    返回一个数组，包含用户信息，*其中phone、realname、studenID等为隐私字段，仅管理员或本人能够获取到此字段(此部分请洽EESAST API 设计者更改之)*
    ```
     [
      {
        "id": 2016011201,
        "username": "huyb",
        "email": "hu-yb16@mails.tsinghua.edu.cn",
        "name": "胡钰彬",
        "phone": 18800123613,
        "department": "电子系",
        "class": "无68",
        "group": "admin",
        "role": "root",
        "createdAt": "2018-12-15T02:49:32.922Z",
        "updatedAt": "2019-01-08T01:09:56.187Z"
      },
      {
        "id": 2016011097,
        "username": "yingrui205",
        "email": "yingrui205@qq.com",
        "name": "应睿",
        "phone": 15652662278,
        "department": "电子系",
        "class": "无64",
        "group": "admin",
        "role": "root",
        "createdAt": "2018-12-14T19:16:17.406Z",
        "updatedAt": "2019-01-08T01:09:58.282Z"
      }
    ]
    ```

没有传入x-access-token或x-access-token非法，状态码401:    
```
    401 Unauthorized: Token required.
    或401 Unauthorized: Invalid or expired token.    
```
* /api/users    POST
  创建新用户

    * header
  | key | value |
  |-------|------|
  | content-type |application/json|
    * body 
    ```
  {
    "id": 0,
    "username": "string",
    "password": "string",
    "email": "string",
    "name": "string",
    "phone": 0,
    "department": "string",
    "class": "string"
  }
    ```
    * response
      若成功创建，返回状态码201，并且返回JSON文本(类似于auth的)，例如:
    ```
        {
            "auth":true,
            "token":"token",
            "id":0,
            "username":"username"
        }
    ```
    *若用户名、邮箱或学号冲突，则状态码返回409(此部分请洽EESAST API 设计者更改之)*
    若注册表单字段不完整，则状态码为422

* /api/users/:id    GET
    * header
    | key | value |
    |-------|------|
    | x-access-token |用户token|
    * response 
    返回JSON字段，包含用户信息，其中phone、realname、studenID等为隐私字段，仅管理员或本人能够获取到此字段
    ```
      {
        "id": 2016011097,
        "username": "yingrui205",
        "email": "yingrui205@qq.com",
        "name": "应睿",
        "phone": 15652662278,
        "department": "电子系",
        "class": "无64",
        "group": "admin",
        "role": "root",
        "createdAt": "2018-12-14T19:16:17.406Z",
        "updatedAt": "2019-01-08T01:09:58.282Z"
      }
    ```
    没有传入x-access-token或x-access-token非法，状态码401:   
``` 
    401 Unauthorized: Token required.
    或401 Unauthorized: Invalid or expired token.    
```
* PUT

  更新用户信息（登陆后才可用）。

  修改其他用户的信息或变更用户组、学号、真实姓名需要管理员权限。

  #### Header

  | Key            | Value            |
  | -------------- | ---------------- |
  | x-access-token | 用户 token       |
  | Content-Type   | application/json |

  #### Body

  只需要写出需要修改的字段。用户名和邮箱不能修改。例如，要修改手机号，则 Body 应如下：

  ```json
  {
    "phone": 12345678901
  }
  ```

  #### Response

  若**更新成功**，则状态码为 `204`。

  若**请求者未传入 `x-access-token` 、 `x-access-token` 非法** 或权限不足，则状态码为 `401`，将返回文本：

  ```
  401 Unauthorized: Token required.
  或
  401 Unauthorized: Invalid or expired token.
  或
  401 Unauthorized: Insufficient permissions.
  ```

  若**用户不存在**，则状态码为 `404`，将返回文本：

  ```
  404 Not Found: User does not exist.
  ```

* DELETE

  删除用户。需要管理员权限。

  #### Header

  | Key            | Value      |
  | -------------- | ---------- |
  | x-access-token | 用户 token |

  #### Response

  若**删除成功**，则状态码为 `204`。

  若**请求者未传入 `x-access-token` 、 `x-access-token` 非法** 或权限不足，则状态码为 `401`，将返回文本：

  ```
  401 Unauthorized: Token required.
  或
  401 Unauthorized: Invalid or expired token.
  或
  401 Unauthorized: Insufficient permissions.
  ```

  若**用户不存在**，则状态码为 `404`，将返回文本：

  ```
  404 Not Found: User does not exist.
  ```




## 队伍系统
| route   | http verb | content |
| ------- | --------- | ------- |
| /api/teams | GET | 获取所有队伍信息 |
| /api/teams | POST | 新增一个队伍|
| /api/teams/:id | GET | 获取对应id队伍的信息|
| /api/teams/:id | PUT | 更新对应id队伍的信息|
| /api/teams/:id | DELETE | 删除对应id的队伍|
| /api/teams/:id/members | GET | 获取相应id队伍的成员|
| /api/teams/:id/members | POST | 加入相应队伍 |
| /api/teams/:id/members/:uid | DELETE | 删除相应id队伍中相应uid成员|
| /api/teams/:id/fight/:uid | POST | 对应id的队伍向对应uid的队伍发起对战 |


##### 此段基本参照之前的组队系统，新增加比赛结果
* /api/teams或者/api/teams/:id    GET
    获取队伍信息，按照id排列

    * header 
    | key | value |
    |-------|------|
    | x-access-token |用户token|
    - parameter 

    | key        | value                |
    | ---------- | -------------------- |
    | detailInfo | boolean (true/false) |

    * response
      若请求成功，返回状态码200和JSON格式文本:
      (获取多个队伍信息时/api/teams，将构成一个数组返回)
      以下为获取单个队伍信息的示例
    ```
    {
        "teamid":0,
        "teamname":"萌新观光团",
        "description":"这是萌新的队伍",
        "members":[0,1,2],//队伍成员的用户id
        "captain":0,//队长的用户id
        "createAt":"Dec. 1,2018,8:38 p.m.",//队伍创建时间
        "invitecode":"123abcde",//随机生成的8位邀请码(隐私字段)
        "score":70,//当前得分
        "rank":1,//当前排名
        //以下内容在/api/teams时可以不返回
        "history-active":[
            {
                enemy:1,//对手队伍id，下同
                result:"lose",//挑战结果，下同
                score:20//最终得分，下同
            },
             {
                enemy:1,
                 result:"win",
                score:40
            },
        ],//主动发起挑战历史
        "history-passive":[
            {
                enemy:2,
                result:"lose",
                score:10
            }
        ]//被挑战历史
    }
    ```

    ##### 注意:仅本队队员可以获取本队invitecode
    ##### 注意:当以GET方式调用/api/teams时可以不返回作战历史
    若token失效，返回401

* /api/teams/:id/members GET
    返回特定id队伍的所有队员(包括队长)

    * header 
        | key | value |
        |-------|------|
        | x-access-token |用户token|
    * response
    若请求成功，返回状态码200和JSON文本(数组):
    ```
    [0,1,2]
    ```
    若队伍不存在，返回状态码404和JSON文本:
    ```
    404 Not Found: Team does not exist.
    ```
    若token失效，返回状态码401

* /api/teams POST
    创建新队伍
    * header 
        | key | value |
        |-------|------|
        | x-access-token |用户token|
        | content-type | application/json|
    * body
    ```
    {
        "teamname":"萌新观光团",
        "description":"这个人很懒，没有留下描述"
    }
    ```
    * response
    若创建成功，返回状态码201和JSON格式的文本:
    ```
    {
        "invitecode":"123abcde"//返回邀请码
    }
    ```
    若队伍名冲突或者请求者已经加入队伍，返回状态码409和JSON文本:
    ```
    409 Conflict: Team name already exists.
    或
    409 Conflict: User is already in a team.
    ```
    若字段缺失或非法，则状态码为422，返回JSON格式的错误信息:
    ```
    {
    "messages": ["Path `name` is required."]
    }
    ```
    若token失效或非法，返回401

* /api/teams/:id/members POST
    加入相应队伍

    * header 
        | key | value |
        |-------|------|
        | x-access-token |用户token|
    * query
        | key | value |
        |-------|------|
        | invitecode |8位字符串|
    * response
    若加入成功，则状态码返回201.
    若请求者已加入队伍，则返回状态码409和JSON文本:
    ```
    409 Conflict: User is already in a team.
    ```
    若缺少邀请码字段，返回状态码422和JSON文本:
    ```
    422 Unprocessable Entity: Missing essential post data.
    ```
    若邀请码错误，则返回状态码403和JSON文本:
    ```
    403 Forbidden: Incorrect invite code.
    ```
    若队伍人数超过上限，则返回状态码409和JSON文本:
    ```
    409 Conflict: The number of members exceeds.
    ```
    若队伍不存在，返回状态码404和JSON文本:
    ```
    404 Not Found: Team does not exist.
    ```
    若token失效或非法，返回状态码401

* /api/teams/:id/members PUT
    修改相应id的队伍的信息，仅队长或管理员可以操作

    * header 
        | key | value |
        |-------|------|
        | x-access-token |用户token|
        | content-type | application/json |
    * body
    只需要写出需要修改的字段。邀请码不能修改（默认只能修改队名及/或队伍描述）
    ```
    {
        "title":"myNewName"
        "description":"myNewDescription"
    }
    ```
    * response
    若修改成功，返回状态码204
    若token失效或非法，返回401
    若队伍不存在，返回状态码404和JSON文本:
    ```
    404 Not Found: Team does not exist.
    ```
    若不是该队队长亦非管理员，返回状态码400和JSON文本:
    ```
    400 Bad Request: The user is neither the captain of the team nor the admin.
    ```

* /api/teams/:id DELETE
    删除相应id的队伍，仅队长可以操作

    * header 
        | key | value |
        |-------|------|
        | x-access-token |用户token|
    * response
    若删除成功，返回状态码204
    若token失效或非法或权限不足，返回401
    若队伍不存在，返回404和JSON文本:
    ```
    404 Not Found: No such team.
    ```

    ​	若不是该队队长亦非管理员，返回状态码400和JSON文本:

    ```
    400 Bad Request: The user is neither the captain of the team nor the admin.
    ```

* /api/teams/:id/members/:uid DELETE
    删除相应id的队伍中的uid成员，队长可以删除本队成员，队员可以自己退出队伍

    * header 
        | key | value |
        |-------|------|
        | x-access-token |用户token|
    * response
    若删除成功，返回状态码204
    若token失效、非法或权限不足，返回状态码401
    若队伍不存在或成员不存在，返回状态码404和JSON文本:
    ```
    404 Not Found: Team does not exist.
    或
    404 Not Found: Member does not exist.
    ```
    若试图删除队长，则返回状态码400和JSON文本:
    ```
    400 Bad Request: Captain cannot be deleted.
    ```

* /api/teams/:id/fight/:uid POST
    对应id的队伍向对应uid的队伍发起一次作战（后端需要记录对应id的history-active和对应uid的history-passive）
    * header 
        | key | value |
        |-------|------|
        | x-access-token |用户token|
    * response
    若请求成功，则返回作战成功后的结果，状态码返回201，同时返回JSON文本:
    ```
    {
        result:"lose",
        score:10
    }
    ```
    若token失效或非法，返回状态码401

## 代码部分
| route | http verb | content |
| ----- |----------|---------|
| /api/codes/teams/:id | POST | 对应id的队伍上传代码(会覆盖之前的) |
| /api/codes/teams/:id | GET |对应id的队伍获取本队代码|

* /api/codes/teams/:id    POST
    * header
    | key | value |
    |-------|------|
    | x-access-token |用户token|
    | content-type | multipart/form-data |
    * body 
    ```
    formdata(input type="file"获取到的本地文件)
    ```
    * response
    如果队员（队长）是在对应id的队伍中，并且上传成功，返回状态码204
    如果上传失败(例如用户不在对应id的队伍中)，返回状态码401
    返回文本：

    ```
    401 Unauthorized: You are not in this team.
    ```
* /api/codes/teams/:id    GET
    返回相应id队伍的文件路径（下载地址）
    * header
    | key | value |
    |-------|------|
    | x-access-token |用户token|
    * response
    若队员（队长）在相应id队伍中，并且请求成功，返回状态码200，和对应代码下载地址(JSON格式):
    ```
    {
        url:"https://xxx",
    }
    ```



## 公告部分
| route | http verb | content |
| ----- | --------- | ------- |
| /api/announce/list | GET | 获取全部的公告信息(标题、时间，不含正文) |
| /api/announce/view/:id | GET | 获取对应id的公告信息(包含标题、时间、正文) |
* /api/announce/list  GET
    列出所有公告的标题、发布时间和最后更新时间

    * response
    请求成功，返回状态码200和JSON文本(数组):
    ```
    [
        {
            "id":0,
            "title":"这是一个公告",
            "pub_date":"Dec. 1,2018,8:38 p.m.",
            "last_update_date":"Dec. 1,2018,8:38 p.m."
        },
        {
        	"id":1,
            "title":"这也是一个公告",
            "pub_date":"Dec. 1,2018,8:38 p.m.",
            "last_update_date":"Dec. 1,2018,8:38 p.m."
        },
    ]
    ```

* /api/announce/view/:id  GET
    获取该id公告的具体内容

    * response
    若请求成功，返回状态码200和JSON文本:
    ```
    {
        "id":0,
        "title":"这是一个公告",
        "content":"这只是一个公告，还没有什么内容"
        "pub_date":"Dec. 1,2018,8:38 p.m.",
        "last_update_date":"Dec. 1,2018,8:38 p.m."
    }
    ```


    若找不到公告，返回状态码404和文本:
    
    404 Not found.

## 文件部分

| route                  | http verb | content                                    |
| ---------------------- | --------- | ------------------------------------------ |
| /api/file/list         | GET       | 获取全部的文件信息(ID、标题、最后更新时间) |
| /api/file/download/:id | GET       | 获取对应id的文件(以文件下载型式传送)       |

- /api/file/list  GET
  获取全部的文件信息(ID、标题、最后更新时间)

  - response
    请求成功，返回状态码200和JSON文本(数组):

  ```
  [
  	{
  		"last_update_date": "2019-03-13T17:00:32.490Z",
  		"title": "\u898f\u5247", 
  		"id": 1
  	}
  ]
  ```

- /api/announce/view/:id  GET
  获取对应id的文件(以文件下载型式传送)

  - response
    - application/octet-stream
    - filename：原始文件名

## 全域设定读取

| route       | http verb | content                                                      |
| ----------- | --------- | ------------------------------------------------------------ |
| /api/global | GET       | 获取全部的全域设定(活动年度、竞赛主题、代码提交的开始与截止时间) |

- /api/global  GET
  获取全部的全域设定(活动年度、竞赛主题、代码提交的开始与截止时间)

  - response
    请求成功，返回状态码200和JSON文本(数组):

  ```
  {
      "year": 2019,
      "game_name": "槍林彈雨",
      "submission_start": "2019-03-14T04:00:00Z",
      "submission_end": "2019-04-18T04:00:00Z"
  }
  ```