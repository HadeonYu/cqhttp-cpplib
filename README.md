## 项目介绍
这是为go-cqhttp开发的第三方C++库，封装了Bot账号，好友信息等api。目前在ubuntu22.04下测试通过，其他平台目前未测试。

## 项目使用
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

# 添加cqhttp-cpp-lib
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

  std::cout << msg->content << std::endl;
  std::cout << msg->senderName << std::endl;
  return 0;
}
```
编译并运行得到输出：
```bash
example/build❯ ./example
[2023-09-18 15:46:40.723] [Bot] [info] Bot Starting Up
[2023-09-18 15:46:40.724] [Bot] [info] Post收到有效响应
{
  "nickname": "Hadeon-qwen",
  "user_id": 2871416104
}
🤔
Hadeon
[2023-09-18 15:47:45.776] [Bot] [info] Bot Shutting Down
```

## 具体使用示例
### Bot账号

#### 1、获取登录账号的信息
```cpp
auto resp = myBot.getLoginInfo();
if (resp->valid) {
  std::cout << *resp << std::endl;
}
```

……待完成