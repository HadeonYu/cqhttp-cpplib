#pragma once

#include "Msg.hpp"
#include "fmt/core.h"
#include "spdlog/spdlog.h"
#include <arpa/inet.h>
#include <deque>
#include <spdlog/sinks/stdout_color_sinks.h>

using spdlog::logger;
using spdlog::set_level;
using spdlog::sinks::stdout_color_sink_st;
using std::make_shared;

#define PRIVATE_MSG "private"
#define GROUP_MSG "group"

namespace cqhttp {

using value_t = std::string;

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
  void printValid(Response *resp); // 输出请求得到的响应的有效情况
public: // TODO 默认构造函数，拷贝构造函数，赋值运算符重载
  Bot(const char *_ip, const uint32_t _listenPort,
      const uint32_t _postPort); // ip地址，监听端口，发送端口
  ~Bot();
  ListenMsg *receive(); // 接受监听消息

  //  Bot账号api:
  Response *getLoginInfo(); // 获取已登录的账号信息
  // 查询在线机型
  Response *getModel(param params);
  Response *getModel(value_t model);

  // 设置在线机型
  Response *setModel(param params);
  Response *setModel(value_t model, value_t model_show);

  // 设置登录号资料
  Response *setProfile(param params);
  Response *setProfile(value_t nickName, value_t company, value_t email,
                       value_t college, value_t personalNote);

  // 获取企点账号信息
  Response *qidianAccountInfo();

  // 获取账号在线客户端列表
  Response *getOnlineClient(param params);
  Response *getOnlineClient(value_t noCache_str);

  // 好友信息api:
  // 获取账号信息
  /* go-cqhttp的帮助文档中
   * 这个api的作用是获取陌生人信息
   * 但是我测试时发现也可以获取好友信息
   */
  Response *getAccountInfo(param params);
  Response *getAccountInfo(value_t userId_str);
  Response *getAccountInfo(value_t userId_str, value_t noCache_str);

  // 获取好友列表
  Response *getFriendList();

  // 获取单向好友列表
  Response *getUnidrectionalList();

  // 好友操作api:
  //  删除好友
  Response *deleteFriend(param params);
  Response *deleteFriend(value_t userId_str);

  // 删除单项好友
  Response *deleteUnidirectional(param params);
  Response *deleteUnidirectional(value_t userId_str);

  // 消息api:
  // 发送私聊消息
  Response *sendPrivateMsg(param params);
  Response *sendPrivateMsg(value_t userId_str, value_t message);
  Response *sendPrivateMsg(value_t userId_str, value_t groupId_str,
                           value_t message);

  // 发送群聊消息
  Response *sendGroupMsg(param params);
  Response *sendGroupMsg(value_t groupId_str, value_t message);

  // 发送消息
  Response *sendMsg(param params);
  Response *sendMsg(value_t msgType, value_t id_str, value_t message);

  // 获取消息信息
  Response *getMsgInfo(param params);
  Response *getMsgInfo(value_t msgId_str);

  // 撤回消息
  Response *deleteMsg(param params);
  Response *deleteMsg(value_t msgId_str);

  // 标记消息已读
  Response *markMsgRead(param params);
  Response *markMsgRead(value_t msgId_str);

  // TODO 合并转发

  // 获取群消息历史记录
  Response *getGroupHistoryMsg(param params);
  Response *getGroupHistoryMsg(value_t msgId_str, value_t groupId_str);
};
} // namespace cqhttp
