#include "Bot.hpp"
#include "Msg.hpp"
#include <string>

namespace cqhttp {
Bot::Bot(const char *_ip, const uint32_t _listenPort, const uint32_t _postPort)
    : listenPort(_listenPort), postPort(_postPort) {
  ip = new char[strlen(_ip)];
  strcpy(ip, _ip);
  msgPost = MsgPost(ip, postPort);
  bufferSize = 4096;
  buffer = new char[bufferSize];
  msgQueMaxSize = 512;
  // 创建logger
  auto consoleSink = make_shared<stdout_color_sink_st>();
  botLogger = new logger("Bot", {consoleSink});
  botLogger->set_level(spdlog::level::debug);
  // 监听端口设置
  setListen();
  // post端口设置
  setPost();
  botLogger->info("Bot Starting Up");
}

Bot::~Bot() {
  botLogger->info("Bot Shutting Down");
  delete botLogger;
  delete buffer;
  close(listenSock);
  close(postSock);
}

void Bot::setListen() {
  listenSock = socket(AF_INET, SOCK_STREAM, 0);
  if (listenSock == -1) {
    botLogger->error("Error Creating Listen Socket:{}", strerror(errno));
    exit(-1);
  }
  // botLogger->info("Listen Socket Created");
  int opt = 1;
  setsockopt(listenSock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

  struct sockaddr_in serverSoc;
  serverSoc.sin_family = AF_INET;
  serverSoc.sin_addr.s_addr = INADDR_ANY;
  serverSoc.sin_port = htons(listenPort);
  if (bind(listenSock, (struct sockaddr *)&serverSoc, sizeof(serverSoc)) ==
      -1) {
    botLogger->error("Error Bind Listen Socket:{}", strerror(errno));
    exit(-1);
  }
  // botLogger->info("Listen Socket Binded");
  if (listen(listenSock, 10) < 0) {
    close(listenSock);
    botLogger->error("Error Listen:{}", strerror(errno));
  }
}

void Bot::setPost() {
  postSock = socket(PF_INET, SOCK_STREAM, 0);
  if (postSock == -1) {
    botLogger->error("Error Creating Post Socket:{}", strerror(errno));
    exit(-1);
  }
  // botLogger->info("Post Socket Created");
  struct sockaddr_in clientSoc;
  clientSoc.sin_family = PF_INET;
  clientSoc.sin_port = htons(postPort);
  clientSoc.sin_addr.s_addr = inet_addr(ip);
  if (connect(postSock, (struct sockaddr *)&clientSoc, sizeof(clientSoc)) ==
      -1) {
    botLogger->error("Error Connect Post Port:{}", strerror(errno));
    exit(-1);
  }
  // botLogger->info("Post Socket Connected");
}

Response *Bot::postRequest(std::string postMsg) {
  if (send(postSock, postMsg.c_str(), postMsg.length(), 0) == -1) {
    botLogger->error("Error to Requset:{}", strerror(errno));
    exit(-1);
  }
  memset(buffer, 0, bufferSize);
  if (recv(postSock, buffer, bufferSize, 0) == -1) {
    botLogger->error("Error to Get Response:{}", strerror(errno));
  }
  Response *resp = new Response(buffer);
  printValid(resp);
  return resp;
}

void Bot::printValid(Response *resp) {
  if (resp->valid) {
    botLogger->info("Post收到有效响应");
  } else {
    botLogger->warn("Post收到无效响应\nhttp状态码：{}；错误信息：{}",
                    resp->statusCode, resp->body["message"].dump());
  }
}

ListenMsg *Bot::receive() {
  memset(buffer, 0, bufferSize);
  strcpy(buffer, "{\"post_type\":\"undefined\"}");
  ListenMsg *listenMsg = new ListenMsg(buffer);
  // 消息有效，且不是重复消息
  bool idInQue = false;
  uint32_t msgId;
  while (!listenMsg->isValid() || idInQue) {
    memset(buffer, 0, bufferSize);
    int clientSock = accept(listenSock, NULL, NULL);
    recv(clientSock, buffer, bufferSize - 1, 0);
    if (!strlen(buffer)) {
      continue;
    }
    delete listenMsg;
    listenMsg = new ListenMsg(buffer);
    //   更新it，用于判断是否重复收到消息
    if (listenMsg->isValid()) {
      msgId = listenMsg->msgId;
      for (uint32_t id : msgIdQue) {
        if (id == msgId) {
          idInQue = true;
          break;
        }
        idInQue = false;
      }
    }
  }
  // 新消息加入队列，若队列长度超过限制则弹出旧消息
  msgIdQue.push_front(msgId);
  if (msgIdQue.size() > msgQueMaxSize) {
    msgIdQue.pop_back();
  }
  if (!strcmp(listenMsg->msgType.c_str(), "private")) {
    botLogger->info("收到好友{}的私聊消息：{}", listenMsg->senderName,
                    listenMsg->content);
  } else if (!strcmp(listenMsg->msgType.c_str(), "group")) {
    botLogger->info("收到来自{}的群聊消息：{}", listenMsg->senderName,
                    listenMsg->content);
  }
  return listenMsg;
}

Response *Bot::getLoginInfo() {
  std::string msg = msgPost.makeMsg("/get_login_info");
  return postRequest(msg);
}

Response *Bot::getModel(param params) {
  std::string msg = msgPost.makeMsg("/_get_model_show", params);
  return postRequest(msg);
}

Response *Bot::getModel(value_t model) {
  param params = {
      {"model", model},
  };
  return getModel(params);
}

Response *Bot::setModel(param params) {
  std::string msg = msgPost.makeMsg("/_set_model_show", params);
  return postRequest(msg);
}

Response *Bot::setModel(value_t model, value_t model_show) {
  param params = {
      {"model", model},
      {"model_show", model_show},
  };
  return setModel(params);
}

Response *Bot::setProfile(param params) {
  std::string msg = msgPost.makeMsg("/set_qq_profile", params);
  return postRequest(msg);
}

Response *Bot::setProfile(value_t nickName, value_t company, value_t email,
                          value_t college, value_t personalNote) {
  param params = {
      {"nickname", nickName},
      {"company", company},
      {"email", email},
      {"college", college},
      {"personal_note", personalNote},
  };
  return setProfile(params);
}

Response *Bot::qidianAccountInfo() {
  std::string msg = msgPost.makeMsg("/qidian_get_account_info");
  return postRequest(msg);
}

Response *Bot::getOnlineClient(param params) {
  std::string msg = msgPost.makeMsg("/get_online_clients", params);
  return postRequest(msg);
}

Response *Bot::getOnlineClient(value_t noCache_str) {
  param params = {
      {"no_cache", noCache_str},
  };
  return getOnlineClient(params);
}

Response *Bot::getAccountInfo(param params) {
  std::string msg = msgPost.makeMsg("/get_stranger_info", params);
  return postRequest(msg);
}

Response *Bot::getAccountInfo(value_t userId_str) {
  param params = {
      {"user_id", userId_str},
      {"no_cache", "false"},
  };
  return getAccountInfo(params);
}

Response *Bot::getAccountInfo(value_t userId_str, value_t noCache_str) {
  param params = {
      {"user_id", userId_str},
      {"no_cache", noCache_str},
  };
  return getAccountInfo(params);
}

Response *Bot::getFriendList() {
  std::string msg = msgPost.makeMsg("/get_friend_list");
  return postRequest(msg);
}

Response *Bot::getUnidrectionalList() {
  std::string msg = msgPost.makeMsg("/get_unidirectional_friend_list");
  return postRequest(msg);
}

Response *Bot::deleteFriend(param params) {
  std::string msg = msgPost.makeMsg("/delete_friend", params);
  return postRequest(msg);
}
Response *Bot::deleteFriend(value_t userId_str) {
  param params = {
      {"user_id", userId_str},
  };
  return deleteFriend(params);
}

Response *Bot::deleteUnidirectional(param params) {
  std::string msg = msgPost.makeMsg("/delete_unidirectional_friend", params);
  return postRequest(msg);
}
Response *Bot::deleteUnidirectional(value_t userId_str) {
  param params = {
      {"user_id", userId_str},
  };
  return deleteUnidirectional(params);
}

Response *Bot::sendPrivateMsg(param params) {
  std::string msg = msgPost.makeMsg("/send_private_msg", params);
  return postRequest(msg);
}

Response *Bot::sendPrivateMsg(value_t userId_str, value_t message) {
  param params = {
      {"user_id", userId_str},
      {"message", message},
  };
  return sendPrivateMsg(params);
}

Response *Bot::sendPrivateMsg(value_t userId_str, value_t groupId_str,
                              value_t message) {
  param params = {
      {"user_id", userId_str},
      {"group_id", groupId_str},
      {"message", message},
  };
  return sendPrivateMsg(params);
}

Response *Bot::sendGroupMsg(param params) {
  std::string msg = msgPost.makeMsg("/send_group_msg", params);
  return postRequest(msg);
}
Response *Bot::sendGroupMsg(value_t groupId_str, value_t message) {
  param params = {
      {"group_id", groupId_str},
      {"message", message},
  };
  return sendGroupMsg(params);
}

Response *Bot::sendMsg(param params) {
  std::string msg = msgPost.makeMsg("/send_msg", params);
  return postRequest(msg);
}
Response *Bot::sendMsg(value_t msgType, value_t id_str, value_t message) {
  param params = {
      {"message_type", msgType},
      {"message", message},
  };
  if (!strcmp(msgType.c_str(), "private")) {
    params["user_id"] = id_str;
  } else if (!strcmp(msgType.c_str(), "group")) {
    params["group_id"] = id_str;
  } else {
    botLogger->warn("消息类型错误！");
    Response *resp = new Response();
    return resp;
  }
  return sendMsg(params);
}

Response *Bot::getMsgInfo(param params) {
  std::string msg = msgPost.makeMsg("/get_msg", params);
  return postRequest(msg);
}
Response *Bot::getMsgInfo(value_t msgId_str) {
  param params = {
      {"message_id", msgId_str},
  };
  return getMsgInfo(params);
}

Response *Bot::deleteMsg(param params) {
  std::string msg = msgPost.makeMsg("/delete_msg", params);
  return postRequest(msg);
}
Response *Bot::deleteMsg(value_t msgId_str) {
  param params = {
      {"message_id", msgId_str},
  };
  return deleteMsg(params);
}

Response *Bot::markMsgRead(param params) {
  std::string msg = msgPost.makeMsg("/mark_msg_as_read", params);
  return postRequest(msg);
}
Response *Bot::markMsgRead(value_t msgId_str) {
  param params = {
      {"message_id", msgId_str},
  };
  return markMsgRead(params);
}

Response *Bot::getGroupHistoryMsg(param params) {
  std::string msg = msgPost.makeMsg("/get_group_msg_history", params);
  return postRequest(msg);
}
Response *Bot::getGroupHistoryMsg(value_t msgId_str, value_t groupId_str) {
  param params = {
      {"message_id", msgId_str},
      {"group_id", groupId_str},
  };
  return getGroupHistoryMsg(params);
}
} // namespace cqhttp
