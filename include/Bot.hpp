#pragma once

#include "Msg.hpp"
#include "spdlog/spdlog.h"
#include <algorithm>
#include <arpa/inet.h>
#include <deque>
#include <memory>
#include <netinet/in.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <sys/socket.h>
#include <sys/types.h>

using spdlog::logger;
using spdlog::set_level;
using spdlog::sinks::stdout_color_sink_st;
using std::make_shared;

namespace cqhttp {
class Bot {
private:
  char *ip;            // 项目所在的ip
  uint32_t listenPort; // 监听端口
  uint32_t postPort;   // 发送端口
  int listenSock;      // 监听套接字
  int postSock;        // 发送套接字
  logger *botLogger;   // 日志输出
  char *buffer;        // 中间变量
  int bufferSize;      // 中间变量buffer的大小
  MsgPost msgPost;     // 用于发送POST
  /*下面的deque msgId用于存储收到的消息id
   * go-cqhttp的部分版本有多次 post 同一条消息的 bug
   * 如果收到的消息已存在于msdId，这条消息会被忽略
   * 否则消息被加入队列头
   * 队列长度超过最大值时，队列尾的元素被弹出
   */
  std::deque<uint32_t> msgIdQue;
  int msgQueMaxSize;                          // msgId的最大长度
  void setListen();                           // 设置监听socket
  void setPost();                             // 设置发送socket
  Response *postRequest(std::string postMsg); // 发送请求
  void valid(Response *resp); // 输出请求得到的响应的有效情况
public: // TODO 默认构造函数，拷贝构造函数，赋值运算符重载
  Bot(const char *_ip, const uint32_t _listenPort,
      const uint32_t _postPort); // ip地址，监听端口，发送端口
  ~Bot();
  ListenMsg *receive(); // 接受监听消息

  // Bot账号api:
  Response *getLoginInfo();                // 获取已登录的账号信息
  Response *getModel(param params);        // 查询在线机型
  Response *setModel(param params);        // 设置在线机型
  Response *getOnlineClient(param params); // 获取账号在线客户端列表

  // 消息api:
  Response *sendPrivateMsg(param params);
};
} // namespace cqhttp
