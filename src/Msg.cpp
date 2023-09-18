#include "Msg.hpp"
#include "fmt/core.h"
#include <ostream>
#include <string>

namespace cqhttp {

// MsgPost类:

MsgPost::MsgPost() : ip(NULL), postPort(0), postTemplate("") {}

MsgPost::MsgPost(char *_ip, uint32_t _postPort) : postPort(_postPort) {
  ip = new char[strlen(_ip)];
  strcpy(ip, _ip);
  postTemplate = "POST {} HTTP/1.1\r\n"
                 "Host: {}:{}\r\n"
                 "Content-Type: application/x-www-form-urlencoded\r\n"
                 "Content-Length: 0\r\n"
                 "\r\n";
}

MsgPost::MsgPost(const MsgPost &cpy)
    : postPort(cpy.postPort), postTemplate(cpy.postTemplate) {
  ip = new char[strlen(cpy.ip)];
  strcpy(ip, cpy.ip);
}

void MsgPost::operator=(const MsgPost &cpy) {
  ip = new char[strlen(cpy.ip)];
  strcpy((char *)ip, cpy.ip);
  postTemplate = cpy.postTemplate;
  postPort = cpy.postPort;
}

MsgPost::~MsgPost() { delete ip; }

std::string MsgPost::makeMsg(const char *endpoint) {
  std::string request =
      fmt::format(postTemplate.c_str(), endpoint, ip, postPort);
  return request;
}

std::string MsgPost::makeMsg(const char *endpoint, param params) {
  std::string endpWithParam(endpoint);
  endpWithParam += "?";
  for (auto &[key, value] : params.items()) {
    endpWithParam += (key + "=" + value.template get<std::string>() + "&");
  }
  endpWithParam.pop_back(); // 删去最后一个多余的'&'
  std::string request =
      fmt::format(postTemplate.c_str(), endpWithParam, ip, postPort);
  return request;
}

// Response类：

Response::Response(char *message) : rawMsg(message) {
  getStatus();
  if (statusCode == 200) {
    // 在message中找到json格式字符串的位置
    const char *start = strchr(message, '{');
    char *nextEnd = NULL;
    char *end = strchr(message, '}');
    while ((nextEnd = strchr(end + 1, '}')) != nullptr) {
      end = nextEnd;
    }
    std::string jsonPart(start, end - start + 1);
    body = json::parse(jsonPart);
    if (body["data"].is_null()) {
      valid = false;
    } else {
      valid = true;
    }
  } else {
    valid = false;
    // 找到rawMsg的最后一行（状态码和错误描述）作为body["message"]
    char *lastBreak = NULL;
    int len = strlen(rawMsg);
    for (int i = len - 1; i > 0; i--) {
      if (rawMsg[i] == '\n') {
        lastBreak = rawMsg + i;
        break;
      }
    }
    lastBreak++;
    body["message"] = lastBreak;
  }
}

Response::Response(const Response &cpy) : body(cpy.body) {
  rawMsg = new char[strlen(cpy.rawMsg)];
  strcpy(rawMsg, cpy.rawMsg);
}

void Response::operator=(const Response &cpy) {
  body = cpy.body;
  rawMsg = new char[strlen(cpy.rawMsg)];
  strcpy(rawMsg, cpy.rawMsg);
}

Response::~Response() { delete rawMsg; }

void Response::getStatus() {
  // 状态码出现在第一个空格和第二个空格之间
  char *blank = strchr(rawMsg, ' ');
  std::string statusStr(blank + 1, 3);
  statusCode = std::stoi(statusStr);
}

std::ostream &operator<<(std::ostream &os, const Response &resp) {
  os << resp.body["data"].dump(2);
  return os;
}

// ListenMsg类：

ListenMsg::ListenMsg() {
  postType = "undefined";
  msgType = "undefined";
}

ListenMsg::ListenMsg(char *message) {
  const char *start = strchr(message, '{');
  const char *end = message + strlen(message);
  std::string jsonPart(start, end - start);
  body = json::parse(jsonPart);
  postType = body["post_type"].template get<std::string>();
  if (isValid()) {
    setInfo();
  }
}

ListenMsg::ListenMsg(const ListenMsg &cpy) { copyFunc(cpy); }

void ListenMsg::operator=(const ListenMsg &cpy) { copyFunc(cpy); }

std::ostream &operator<<(std::ostream &os, const ListenMsg &msg) {
  os << msg.body.dump(2);
  return os;
}

void ListenMsg::setInfo() {
  msgType = body["message_type"].template get<std::string>();
  subType = body["sub_type"].template get<std::string>();
  content = body["message"].template get<std::string>();
  senderName = body["sender"]["nickname"].template get<std::string>();
  msgId = body["message_id"].template get<uint32_t>();
  userId = body["user_id"].template get<int64_t>();
  time = body["time"].template get<int64_t>();
  if (!strcmp(msgType.c_str(), "group")) {
    groupId = body["group_id"].template get<int64_t>();
  } else {
    groupId = 0;
  }
}

void ListenMsg::copyFunc(const ListenMsg &cpy) {
  postType = cpy.postType;
  msgType = cpy.msgType;
  subType = cpy.subType;
  content = cpy.content;
  senderName = cpy.senderName;
  msgId = cpy.msgId;
  userId = cpy.userId;
  time = cpy.time;
  groupId = cpy.groupId;
}

bool ListenMsg::isValid() {
  /*
   * 如果post类型为message则有效
   * 希望其他类型也有效可在这里修改
   * post_type具体说明：https://docs.go-cqhttp.org/reference/data_struct.html#post-type
   */
  bool isMsg = !strcmp(postType.c_str(), "message");
  // bool ……
  return isMsg;
  // return isMsg && ……；
}

} // namespace cqhttp
