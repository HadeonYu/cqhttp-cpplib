#include "Bot.hpp"
#include "Msg.hpp"
#include <algorithm>
#include <cstring>
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
  serverSoc.sin_addr.s_addr = inet_addr(ip);
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
  valid(resp);
  return resp;
}

void Bot::valid(Response *resp) {
  std::string status = resp->body["status"];
  if (resp->statusCode == 200 && !strcmp(status.c_str(), "ok")) {
    botLogger->info("Post收到有效响应");
  } else {
    botLogger->warn(
        "Post收到无效响应\n响应状态：{}；http状态码：{}；错误信息：{}", status,
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

Response *Bot::getModel(const char *model) {
  param params = {
      {"model", model},
  };
  return getModel(params);
}

Response *Bot::setModel(param params) {
  std::string msg = msgPost.makeMsg("/_set_model_show", params);
  return postRequest(msg);
}

Response *Bot::setModel(const char *model, const char *model_show) {
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

Response *Bot::setProfile(const char *nickName, const char *company,
                          const char *email, const char *college,
                          const char *personalNote) {
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

Response *Bot::getOnlineClient(const char *noCache) {
  param params = {
      {"no_cache", noCache},
  };
  return getOnlineClient(params);
}

Response *Bot::sendPrivateMsg(param params) {
  std::string msg = msgPost.makeMsg("/send_private_msg", params);
  return postRequest(msg);
}

} // namespace cqhttp
