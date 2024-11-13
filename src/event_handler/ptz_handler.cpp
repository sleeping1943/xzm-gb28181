#include "ptz_handler.h"
#include "../utils/config.h"
#include "fmt/format.h"
#include "src/xzm_defines.h"
#include <array>
#include <memory>

PtzHandler::PtzHandler() {}

PtzHandler::~PtzHandler() {}

void PtzHandler::request_ptz_without_ack(eXosip_t *sip_context,
                                         ClientRequestPtr req) {

  ClientPtr client = req->client_ptr;
  auto param_ptr = req->client_ptr->param_ptr;
  if (!sip_context || !client) {
    return;
  }
  auto s_info = gServerInfo;

  std::string ptz_cmd = generate_ptz_cmd(req);
  LOG(DEBUG) << fmt::format("ptz_cmd:{}", ptz_cmd);
  if (ptz_cmd.empty()) {
    LOG(ERROR) << "ptz_cmd is empty!";
    return;
  }

  std::string str_from = fmt::format("sip:{}@{}:{}", s_info.sip_id.c_str(),
                                     s_info.ip.c_str(), s_info.port);
  std::string str_to = fmt::format("sip:{}@{}:{}", client->device.c_str(),
                                   client->ip.c_str(), client->port);
  std::string str_body =
      fmt::format("<?xml version=\"1.0\"?>"
                  "<Control>"
                  "<CmdType>DeviceControl</CmdType>"
                  "<SN>{}</SN>"
                  "<DeviceID>{}</DeviceID>"
                  "<PTZCmd>{}</PTZCmd>"
                  "<Info>"
                  "<ControlPriority>{}</ControlPriority>"
                  "</Info>"
                  "</Control>",
                  get_random_sn(), client->device.c_str(), ptz_cmd, 5);

  osip_message_t *message = nullptr;
  eXosip_message_build_request(sip_context, &message, "MESSAGE", str_to.c_str(),
                               str_from.c_str(), nullptr);
  osip_message_set_body(message, str_body.c_str(), str_body.size());
  osip_message_set_content_type(message, "Application/MANSCDP+xml");
  eXosip_lock(sip_context);
  int ret = eXosip_message_send_request(sip_context, message);
  LOG(INFO) << fmt::format("send ptz cotrol ret:{}", ret);
  eXosip_unlock(sip_context);
  return;
}

void PtzHandler::request_ptz_with_ack(eXosip_t *sip_context,
                                      ClientRequestPtr req) {
  ClientPtr client = req->client_ptr;
  auto param_ptr = req->client_ptr->param_ptr;
  if (!sip_context || !client) {
    return;
  }
  auto s_info = gServerInfo;

  std::string str_from = fmt::format("sip:{}@{}:{}", s_info.sip_id.c_str(),
                                     s_info.ip.c_str(), s_info.port);
  std::string str_to = fmt::format("sip:{}@{}:{}", client->device.c_str(),
                                   client->ip.c_str(), client->port);
  std::string record_cmd = generate_record_cmd(req);
  std::string str_body =
      fmt::format("<?xml version=\"1.0\"?>"
                  "<Control>"
                  "<CmdType>DeviceControl</CmdType>"
                  "<SN>{}</SN>"
                  "<DeviceID>{}</DeviceID>"
                  "<RecordCmd>{}</RecordCmd>"
                  "</Control>",
                  get_random_sn(), client->device.c_str(), record_cmd, 5);

  osip_message_t *message = nullptr;
  eXosip_message_build_request(sip_context, &message, "MESSAGE", str_to.c_str(),
                               str_from.c_str(), nullptr);
  osip_message_set_body(message, str_body.c_str(), str_body.size());
  osip_message_set_content_type(message, "Application/MANSCDP+xml");
  eXosip_lock(sip_context);
  int ret = eXosip_message_send_request(sip_context, message);
  LOG(INFO) << fmt::format("send ptz cotrol ret:{}", ret);
  eXosip_unlock(sip_context);
  return;
}

// 引用资料: https://blog.csdn.net/www_dong/article/details/133998072
std::string PtzHandler::generate_ptz_cmd(ClientRequestPtr req) {
  std::string str_cmd;
  auto param_ptr =
      std::dynamic_pointer_cast<Xzm::RequestParamPTZ>(req->param_ptr);
  if (!param_ptr) {
    LOG(ERROR) << "can not transform req->param_ptr to RequestParamPTZPtr!";
    return str_cmd;
  }
  std::array<unsigned char, PTZCMDLENGTH> arr_ptz = {0xA5, 0x0F, 0x01, 0x00,
                                                     0x00, 0x00, 0x00, 0x00};
  auto type = param_ptr->cmd_type;
  if (type <= Xzm::kPTZ_CTRL_NONE || type >= Xzm::kPTZ_CTRL_MAX) {
    type = Xzm::kPTZ_CTRL_NONE;
  }
  int value = param_ptr->value;
  switch (type) {
  case Xzm::kPTZ_CTRL_NONE:
    break;
  case Xzm::kPTZ_CTRL_RIGHT:
    arr_ptz[3] = 0x01;
    arr_ptz[4] = value & 0xFF;
    break;
  case Xzm::kPTZ_CTRL_RIGHTUP:
    arr_ptz[3] = 0x09;
    arr_ptz[4] = value & 0xFF;
    arr_ptz[5] = value & 0xFF;
    break;
  case Xzm::kPTZ_CTRL_UP:
    arr_ptz[3] = 0x08;
    arr_ptz[5] = value & 0xFF;
    break;
  case Xzm::kPTZ_CTRL_LEFTUP:
    arr_ptz[3] = 0x0A;
    arr_ptz[4] = value & 0xFF;
    arr_ptz[5] = value & 0xFF;
    break;
  case Xzm::kPTZ_CTRL_LEFT:
    arr_ptz[3] = 0x02;
    arr_ptz[4] = value & 0xFF;
    break;
  case Xzm::kPTZ_CTRL_LEFTDOWN:
    arr_ptz[3] = 0x06;
    arr_ptz[4] = value & 0xFF;
    arr_ptz[5] = value & 0xFF;
    break;
  case Xzm::kPTZ_CTRL_DOWN:
    arr_ptz[3] = 0x04;
    arr_ptz[5] = value & 0xFF;
    break;
  case Xzm::kPTZ_CTRL_RIGHTDOWN:
    arr_ptz[3] = 0x05;
    arr_ptz[4] = value & 0xFF;
    arr_ptz[5] = value & 0xFF;
    break;
  case Xzm::kPTZ_CTRL_ZOOM:
    arr_ptz[3] = (value > 0) ? 0x10 : 0x20;
    arr_ptz[6] = (std::abs(value) & 0x0F) << 4;
    break;
  case Xzm::kPTZ_CTRL_IRIS:
    arr_ptz[3] = (value > 0) ? 0x44 : 0x48;
    arr_ptz[5] = (std::abs(value) & 0xFF) << 4;
    break;
  case Xzm::kPTZ_CTRL_FOCUS:
    arr_ptz[3] = (value > 0) ? 0x44 : 0x48;
    arr_ptz[5] = (std::abs(value) & 0xFF) << 4;
    break;
  case Xzm::kPTZ_CTRL_MAX:
    break;
  default:
    break;
  }
  int index = 0;

  for (const auto &c : arr_ptz) {
    arr_ptz[PTZCMDLENGTH - 1] += c;
    str_cmd += fmt::format("{0:02X}", c);
    if (++index == (arr_ptz.size() - 1)) {
      break;
    }
  }
  str_cmd += fmt::format("{0:02X}", arr_ptz.back());
  return str_cmd;
}

std::string PtzHandler::generate_record_cmd(ClientRequestPtr) {}
