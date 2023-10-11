# 项目介绍
这是为 go-cqhttp 开发的第三方 C++ 库，封装了 Bot 账号，好友信息等 API。目前在 Ubuntu22.04 下测试通过，其他平台目前未测试。

# 项目使用
以CMake为例，假设项目目录结构如下:
```objectivec
project example
├── CMakeLists.txt
├── build
├── include
│   └── cqhttp-cpplib  <--本项目
└── src
  └── main.cpp
```
``CMakeLists.txt``:
```cmake
cmake_minimum_required(VERSION 3.5)
project(example)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")

set(SOURCES src/main.cpp)

add_executable(${PROJECT_NAME} ${SOURCES})
target_include_directories(${PROJECT_NAME} PRIVATE include)

# 添加cqhttp-cpplib
add_subdirectory(include/cqhttp-cpplib)
target_link_libraries(${PROJECT_NAME} PRIVATE cqhttp-cpplib)
```

``src/main.cpp``:
```cpp
#include "cqhttp-cpplib/cqhttp.hpp"
#include <iostream>

#define LISTEN_PORT 5701 // 监听端口，即go-cqhttp的Post端口
#define POST_PORT 5700   // Post端口，即go-cqhttp的监听端口
#define IP "127.0.0.1"   // go-cqhttp所在的ip地址

int main() {
cqhttp::Bot myBot(IP, LISTEN_PORT, POST_PORT);

//获取登录账号的信息
auto resp = myBot.getLoginInfo();
// resp 类型为 Response*

if (resp->valid) {
  std::cout << *resp << std::endl;
}

//接受消息
auto msg = myBot.receive(); // 阻塞，直到监听端口收到消息
// msg 类型为 ListenMsg*

// 消息内容
std::cout << msg->content << std::endl;
// 发送者昵称
std::cout << msg->senderName << std::endl;
return 0;
}
```
编译并运行得到输出：
```bash
example/build❯ ./example
[2023-09-18 16:08:24.857] [Bot] [info] Bot Starting Up
[2023-09-18 16:08:24.858] [Bot] [info] Post收到有效响应
{
"nickname": "Hadeon-qwen",
"user_id": 1234567890
}
[2023-09-18 16:08:49.013] [Bot] [info] 收到好友Hadeon的私聊消息：message receive example🤔
message receive example🤔
Hadeon
[2023-09-18 16:08:49.013] [Bot] [info] Bot Shutting Down
```

# 具体使用示例
封装的函数有两种传参方式：

1. 通过json格式的param传参:
```cpp
cqhttp::param params = {
{"key1", "val1"},
{"key2", "val2"},
...
};
myBot.postFunc(params);
```
``cqhttp::param`` 的本质是 ``nlohmann::json``，更多初始化和参数操作方式请参考项目[nlohmann/json](https://github.com/nlohmann/json)

2. 通过封装好的重载传参：
```cpp
cqhttp::value_t val1 = "value1";
cqhttp::value_t val2 = "value2";
...
myBot.postFunc(val1, val2, ...);
```
**⚠️⚠️⚠️ 注意： 不论哪种方法，所有参数必须是字符串格式！！！**

具体参数和返回参考go-cqhttp[帮助文档](https://docs.go-cqhttp.org/api)
## Bot账号
### 1、获取登录账号的信息
(该API没有参数)
```cpp
auto resp = myBot.getLoginInfo();
```
### 2、设置登录号资料
```cpp
auto resp = 
myBot.setProfile("NickName", "company",     
                  "hadeon@qq.com", "XJTU", 
                  "a qq bot");

// 或者：
cqhttp::param params = {
  {"nickname", "NickName"},      
  {"company", "company"},
  {"email", "hadeon@qq.com"},    
  {"college", "XJTU"},
  {"personal_note", "a qq bot"},
};
auto resp = myBot.setProfile(params);
```
### 3、获取企点账号信息
（该API只有企点协议可用，无参数）
```cpp
auto resp = myBot.qidianAccountInfo();
```
### 4、获取在线机型
```cpp
auto resp = myBot.getModel("string");

// 或者：
cqhttp::param params = {
    {"model", "string"},
};
auto resp = myBot.getModel(params);

```
### 5、设置在线机型
```cpp
auto resp = myBot.setModel(
  "string", 
  "string"
);

// 或者：
cqhttp::param params = {
  {"model", "string"},
  {"model_show", "string"},
  };
auto resp = myBot.setModel(params);
```
### 6、获取当前账号在线客户端列表
```cpp
auto resp = myBot.getOnlineClients("true");

// 或者：
cqhttp::param params = {
    {"no_cache", "false"},
};
auto resp = myBot.getOnlineClient(params);
```

## 好友信息
### 1、获取账号信息
```cpp
auto resp = myBot.getAccountInfo("1234567890", "true");

// 或者：
auto resp = myBot.getAccountInfo("1234567890");

// 或者：
cqhttp::param params = {
    {"user_id", "1234567890"},
    {"no_cache", "false"},
};
auto resp = myBot.getAccountInfo(params);
```

### 2、获取好友列表
（该API没有参数）
```cpp
auto resp = myBot.getFriendList();
```

### 3、获取单向好友列表
(该API没有参数)
```cpp
auto resp = myBot.getUnidrectionalList();
```

### 4、删除好友
```cpp
auto resp = myBot.deleteFriend("1234567890");

// 或者：
cqhttp::param params = {
    {"user_id", "1234567890"},
};
auto resp = myBot.deleteFriend(params);
```

### 5、删除单向好友
```cpp
auto resp = myBot.deleteUnidrectional("1234567890");

// 或者：
cqhttp::param params = {
    {"user_id", "1234567890"},
};
auto resp = myBot.deleteUnidrectional(params);
```
## 消息
### 1、发送私聊消息
```cpp
cqhttp::value_t id = "1234567890", message = "a message";
auto resp = myBot.sendPrivateMsg(id, message);

// 或者：
cqhttp::param params = {
    {"user_id", "1234567890"},
    {"message", "a message"},
};
auto resp = myBot.sendPrivateMsg(params);

//通过群聊发起临时会话：
cqhttp::value_t userId_str, groupId_str, message;
auto resp = myBot.sendPrivateMsg(userId_str, groupId_str, message);
```

### 2、发送群聊消息
```cpp
cqhttp::value_t groupId = "1234567890", message = "a message";
auto resp = myBot.sendGroupMsg(groupId, message);

// 或者：
cqhttp::param params = {
    {"group_id", "1234567890"},
    {"message", "a message"},
};
auto resp = myBot.sendGroupMsg(params);
```
### 3、发送消息
#### （1）发送私聊消息
```cpp
auto resp = myBot.sendMsg(PRIVATE_MSG, "1234567890", "I'm a bot");

// 或者
cqhttp::param params = {
    {"user_id", "1234567890"},
    {"message", "I'm a bot"},
};
resp = myBot.sendMsg(params);
```
#### （2）发送群聊消息
```cpp
auto resp = myBot.sendMsg(GROUP_MSG, "1234567890", "I'm a bot");

// 或者
cqhttp::param params = {
    {"group_id", "1234567890"},
    {"message", "I'm a bot"},
};
resp = myBot.sendMsg(params);
```
### 4、获取消息信息
```cpp
auto msg = myBot.receive(); // 接收消息

cqhttp::value_t msgId = msg->msgId_str; // 获取接受消息的msgId

auto resp = myBot.getMsgInfo(msgId);

// 或者：
cqhttp::param params = {
  {"message_id", msgId},
};
resp = getMsgInfo(params);
```

### 5、撤回消息
```cpp
auto resp = myBot.deleteMsg(msgId);

// 或者：
cqhttp::param params = {
  {"message_id", msgId},
};
resp = deleteMsg(params);
```
### 6、标记消息已读
```cpp
auto resp = myBot.markMsgRead(msgId);

// 或者：
cqhttp::param params = {
  {"message_id", msgId},
};
resp = markMsgRead(params);
```