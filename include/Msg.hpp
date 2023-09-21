#pragma once

#include <cstdint>
#include <fmt/format.h>
#include <iostream>
#include <nlohmann/json.hpp>
#include <ostream>
#include <string>

using json = nlohmann::json;

namespace cqhttp {

using param = nlohmann::json;

class MsgPost { // 用于POST的消息
private:
  std::string postTemplate;
  char *ip;
  uint32_t postPort;

public:
  MsgPost(); // 用于post的消息
  MsgPost(char *_ip, uint32_t _postPort);
  MsgPost(const MsgPost &cpy);
  void operator=(const MsgPost &cpy);
  ~MsgPost();
  std::string makeMsg(const char *endpoint); // 不带参数的请求
  std::string makeMsg(const char *endpoint,
                      param params); // 带参数，参数格式为json
};

class Response { // POST后收到的响应
private:
  void getStatus(); // 获取状态码
  char *rawMsg;     // 直接得到的没处理的信息

public:
  json body;      // 处理得到的json格式信息
  int statusCode; // 状态码
  bool valid;     // 消息是否有效
  Response(char *message);
  Response(const Response &cpy);
  ~Response();
  void operator=(const Response &cpy);
  friend std::ostream &operator<<(std::ostream &os, const Response &resp);
};

class ListenMsg { // 监听端口收到的消息
  // 只接收post_type为message的事件，可在Msg.cpp中,bool ListenMsg::isValid()修改
private:
  void setInfo();                      // 通过body设置各项成员
  void copyFunc(const ListenMsg &cpy); // 拷贝构造函数和赋值运算符重载给成员赋值
public:
  json body;
  ListenMsg();
  ListenMsg(char *message);
  // 如果加了指针类型私有成员，记得修改拷贝构造函数和赋值运算符重载
  ListenMsg(const ListenMsg &cpy);
  void operator=(const ListenMsg &cpy);
  bool isValid(); // 判断收到的消息是否有效
  friend std::ostream &operator<<(std::ostream &os, const ListenMsg &msg);
  std::string postType;   // 上报类型
  std::string msgType;    // 消息类，私聊或群聊
  std::string subType;    // 消息子类型
  std::string content;    // 消息内容
  std::string senderName; // 发送者昵称
  uint32_t msgId;         // 消息id
  int64_t userId;         // 发送者qq号，私聊群聊都有
  int64_t time;           // 消息的时间戳
  // TODO 用继承优化类的结构
  int64_t groupId; // 群聊号，私聊消息值为0
};
} // namespace cqhttp
