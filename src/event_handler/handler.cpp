#include "handler.h"

#include <osipparser2/headers/osip_header.h>
#include <osipparser2/osip_message.h>
#include <osipparser2/osip_parser.h>
#include <string.h>

#include <algorithm>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <cctype>
#include <chrono>
#include <memory>
#include <ostream>
#include <random>

#include "../msg_builder/msg_builder.h"
#include "../server.h"
#include "../utils/chinese.h"
#include "../utils/config.h"
#include "../utils/helper.h"
#include "../utils/log.h"
#include "../utils/tinyxml2.h"
#include "fmt/format.h"

// using tinyxml2::XMLDocument;
using tinyxml2::XMLAttribute;
using tinyxml2::XMLElement;
using tinyxml2::XMLError;

using std::chrono::duration_cast;
using std::chrono::seconds;
using std::chrono::system_clock;

namespace Xzm {

uint64_t Handler::sn_ = 10000;

std::unordered_map<std::string, FUNC_MSG_RESPONSE> msg_response_;

Handler::Handler() {}

Handler::~Handler() {}

bool Handler::Process(eXosip_event_t *evtp, eXosip_t *sip_context_, int code)
{
    is_print = true;
    // std::cout << "Handler Process!!" << std::endl;
    this->response_message(evtp, sip_context_, code);
    if (is_print) {
        this->dump_request(evtp);
        this->dump_response(evtp);
    }
    return true;
}

int Handler::request_cancel_invite(eXosip_t *sip_context_, ClientRequestPtr req)
{
    std::string ssrc = req->ssrc;
    auto info_ids = gServer->FindPublishStreamInfo(ssrc);
    int cid = info_ids.first;
    int did = info_ids.second;
    if (cid < 0 || did < 0) {
        return -1;
    }
    eXosip_lock(sip_context_);
    eXosip_call_terminate(sip_context_, cid, did);
    eXosip_unlock(sip_context_);
    gServer->DelPublishStreamInfo(ssrc);
    return 0;
}

int Handler::request_bye(eXosip_event_t *evtp, eXosip_t *sip_context_)
{
    eXosip_lock(sip_context_);
    eXosip_call_terminate(sip_context_, evtp->cid, evtp->did);
    eXosip_unlock(sip_context_);
    return 0;
}

void Handler::response_message(eXosip_event_t *evtp, eXosip_t *sip_context_, int code)
{
    if (evtp == nullptr || evtp->request == nullptr) {
        LOG(ERROR) << "evtp or evtp->requets is nullptr!";
        return;
    }

    auto cseq_t = osip_message_get_cseq(evtp->request);
    std::string method = cseq_t ? cseq_t->method : "";
    std::transform(method.begin(), method.end(), method.begin(), [](char c) { return std::tolower(c); });
    if (method == "ack" || method == "invite") {  // invite/ack等包没有xml报文,不能解析出cmdtype和deviceid,不响应即可
        return;
    }
    osip_body_t *body = nullptr;
    char CmdType[64] = {0};
    char DeviceID[64] = {0};
    // 获取sip协议中message消息body体xml数据并解析
    osip_message_get_body(evtp->request, 0, &body);
    if (body) {
        parse_xml(body->body, "<CmdType>", false, "</CmdType>", false, CmdType);
        parse_xml(body->body, "<DeviceID>", false, "</DeviceID>", false, DeviceID);
        // CLOGI(YELLOW, "%s", body->body);
    }

    if (Server::is_server_quit) {  // 已经开始关闭服务,删除该客户端,发送bye
        Server::instance()->RemoveClient(DeviceID);
        request_bye(evtp, sip_context_);
        return;
    }
    if (!Server::instance()->IsClientExist(DeviceID) &&
        !Server::instance()->IsClientInfoExist(DeviceID)) {  // 服务器没有此客户端信息,也不是音频通道ID,断开连接
        request_bye(evtp, sip_context_);
        return;
    }

    LOG(INFO) << fmt::format("CmdType={},DeviceID={}", CmdType, DeviceID);
    auto func = Server::instance()->GetMsgResponse(CmdType);
    std::shared_ptr<boost::any> param_ptr = std::make_shared<boost::any>();
    if (func) {
        func(evtp, sip_context_, 200, nullptr);
    } else {
        this->response_message_answer(evtp, sip_context_, 200);  // 默认处理
    }
    // if(!strcmp(CmdType, "Catalog")) {
    //     this->parse_device_xml(body->body);
    //     this->response_message_answer(evtp, sip_context_, 200);
    //     // 需要根据对方的Catelog请求，做一些相应的应答请求
    // } else if(!strcmp(CmdType, "Keepalive")){   // 心跳消息
    //     is_print = false;
    //     this->response_message_answer(evtp, sip_context_, 200);
    // }else{
    //     this->response_message_answer(evtp, sip_context_, 200);
    // }
    return;
}

void Handler::response_message_answer(eXosip_event_t *evtp, eXosip_t *sip_context_, int code)
{
    int returnCode = 0;
    osip_message_t *pRegister = nullptr;
    returnCode = eXosip_message_build_answer(sip_context_, evtp->tid, code, &pRegister);
    bool bRegister = false;
    if (pRegister) {
        bRegister = true;
    }
    if (returnCode == 0 && bRegister) {
        eXosip_lock(sip_context_);
        eXosip_message_send_answer(sip_context_, evtp->tid, code, pRegister);
        eXosip_unlock(sip_context_);
    } else {
        LOG(ERROR) << fmt::format("code={},returnCode={},bRegister={}", code, returnCode, bRegister);
    }
}

void Handler::response_catalog(eXosip_event_t *evtp, eXosip_t *sip_context_, int code,
                               std::shared_ptr<boost::any> param)
{
    osip_body_t *body = nullptr;
    osip_message_get_body(evtp->request, 0, &body);
    parse_device_xml(body->body);
    response_message_answer(evtp, sip_context_, 200);
    return;
}

void Handler::response_recordinfo(eXosip_event_t *evtp, eXosip_t *sip_context_, int code,
                                  std::shared_ptr<boost::any> param)
{
    osip_body_t *body = nullptr;
    osip_message_get_body(evtp->request, 0, &body);
    bool is_last_item = false;
    parse_recordinfo_xml(body->body, is_last_item);
    if (is_last_item) {  // 该设备历史录像获取完毕
        // 通知获取完成,返回前端录像信息
        Server::instance()->NotifyHistoryComplete();
        LOG(ERROR) << "query history completed!!Notify histtory complete!";
    }
    response_message_answer(evtp, sip_context_, 200);
    return;
}

void Handler::response_keepalive(eXosip_event_t *evtp, eXosip_t *sip_context_, int code,
                                 std::shared_ptr<boost::any> param)
{
    is_print = false;
    this->response_message_answer(evtp, sip_context_, 200);
}

void Handler::response_alarm(eXosip_event_t *evtp, eXosip_t *sip_context_, int code, std::shared_ptr<boost::any> param)
{
    osip_body_t *body = nullptr;
    osip_message_get_body(evtp->request, 0, &body);
    parse_device_xml(body->body);
    this->response_message_answer(evtp, sip_context_, 200);
}

int Handler::request_invite(eXosip_t *sip_context, ClientRequestPtr req)
{
    char session_exp[1024] = {0};
    osip_message_t *msg = nullptr;
    char from[1024] = {0};
    char to[1024] = {0};
    char contact[1024] = {0};
    char sdp[2048] = {0};
    ClientPtr client = req->client_ptr;
    /*
        在http请求推流时就确定rtsp推流地址
    */
    auto s_info = gServerInfo;
    auto media_info = gMediaServerInfo;
    // client->ssrc = Xzm::util::build_ssrc(true, s_info.realm);
    // auto ssrc = Xzm::util::convert10to16(client->ssrc);
    // client->rtsp_url = Xzm::util::get_rtsp_addr(s_info.rtp_ip, ssrc);

    LOG(INFO) << fmt::format("addr:{}", client->rtsp_url);
    sprintf(from, "sip:%s@%s:%d", s_info.sip_id.c_str(), s_info.ip.c_str(), s_info.port);
    sprintf(contact, "sip:%s@%s:%d", s_info.sip_id.c_str(), s_info.ip.c_str(), s_info.port);
    sprintf(to, "sip:%s@%s:%d", client->real_device_id.c_str(), client->ip.c_str(), client->port);
    snprintf(sdp, 2048,
             "v=0\r\n"
             "o=%s 0 0 IN IP4 %s\r\n"
             "s=Play\r\n"
             "c=IN IP4 %s\r\n"
             "t=0 0\r\n"
             //"m=video %d TCP/RTP/AVP 96 98 97\r\n"
             "m=video %d RTP/AVP 96 98 97\r\n"
             "a=recvonly\r\n"
             "a=rtpmap:96 PS/90000\r\n"
             "a=rtpmap:98 H264/90000\r\n"
             "a=rtpmap:97 MPEG4/90000\r\n"
             //"a=setup:passive\r\n"
             //"a=connection:new\r\n"
             "y=%s\r\n"
             "f=\r\n",
             client->real_device_id.c_str(), media_info.rtp_ip.c_str(), media_info.rtp_ip.c_str(), media_info.rtp_port,
             client->ssrc.c_str());
    //"y=0100000001\r\n"
    //"f=\r\n", s_info.sip_id.c_str(),s_info.ip.c_str(), s_info.rtp_ip.c_str(),
    // s_info.rtp_port);

    int ret = eXosip_call_build_initial_invite(sip_context, &msg, to, from, nullptr, nullptr);
    if (ret) {
        LOG(ERROR) << fmt::format("eXosip_call_build_initial_invite error: {} {} ret:{}", from, to, ret);
        return -1;
    }

    osip_message_set_body(msg, sdp, strlen(sdp));
    osip_message_set_content_type(msg, "application/sdp");
    snprintf(session_exp, sizeof(session_exp) - 1, "%i;refresher=uac", s_info.timeout);
    osip_message_set_header(msg, "Session-Expires", session_exp);
    osip_message_set_supported(msg, "timer");

    int call_id = eXosip_call_send_initial_invite(sip_context, msg);

    if (call_id > 0) {
        LOG(INFO) << fmt::format("eXosip_call_send_initial_invite success: call_id={}", call_id);
    } else {
        LOG(ERROR) << fmt::format("eXosip_call_send_initial_invite error: call_id={}", call_id);
    }
    return ret;
}

int Handler::request_invite_talk(eXosip_t *sip_context, ClientRequestPtr req)
{
    char session_exp[1024] = {0};
    osip_message_t *msg = nullptr;
    char from[1024] = {0};
    char to[1024] = {0};
    char contact[1024] = {0};
    char sdp[2048] = {0};
    ClientPtr client = req->client_ptr;
    auto media_info = gMediaServerInfo;

    auto s_info = gServerInfo;
    client->ssrc = Xzm::util::build_ssrc(true, s_info.realm);
    auto ssrc = Xzm::util::convert10to16(client->ssrc);
    client->rtsp_url = Xzm::util::get_rtsp_addr(media_info.rtp_ip, ssrc);

    LOG(INFO) << fmt::format("addr:{}", client->rtsp_url);
    sprintf(from, "sip:%s@%s:%d", s_info.sip_id.c_str(), s_info.ip.c_str(), s_info.port);
    sprintf(contact, "sip:%s@%s:%d", s_info.sip_id.c_str(), s_info.ip.c_str(), s_info.port);
    sprintf(to, "sip:%s@%s:%d", client->device.c_str(), client->ip.c_str(), client->port);
    /*
    snprintf (sdp, 2048,
              "v=0\r\n"
              "o=%s 0 0 IN IP4 %s\r\n"
              "s=Talk\r\n"
              "c=IN IP4 %s\r\n"
              "t=0 0\r\n"
              "m=audio %d RTP/AVP 8\r\n"
              "a=sendrecv\r\n"
              "a=rtpmap:8 PCMA/8000\r\n"
              "a=setup:passive\r\n"
              "a=connection:new\r\n"
              "y=%s\r\n"
              "f=v/a/1/8/1=\r\n", client->device.c_str(),s_info.rtp_ip.c_str(),
    s_info.rtp_ip.c_str(), s_info.rtp_port, client->ssrc.c_str());
              //"y=0100000001\r\n"
              //"f=\r\n", s_info.sip_id.c_str(),s_info.ip.c_str(),
    s_info.rtp_ip.c_str(), s_info.rtp_port);
    */
    snprintf(sdp, 2048,
             "v=0\r\n"
             "o=%s 0 0 IN IP4 %s\r\n"
             "s=Play\r\n"
             "c=IN IP4 %s\r\n"
             "t=0 0\r\n"
             "m=audio %d RTP/AVP 8\r\n"
             "a=recvonly\r\n"  // SIP服务器收取音频数据
             "a=rtpmap:8 PCMA/8000\r\n"
             //"a=setup:passive\r\n" // TCP被动模式
             //"a=connection:new\r\n"    // 每次新建连接
             "y=%s\r\n"
             "f=v/////a/1/8/1\r\n",
             client->device.c_str(), media_info.rtp_ip.c_str(), media_info.rtp_ip.c_str(), media_info.rtp_port,
             client->ssrc.c_str());
    //"y=0100000001\r\n"
    //"f=\r\n", s_info.sip_id.c_str(),s_info.ip.c_str(), s_info.rtp_ip.c_str(),
    // s_info.rtp_port);
    // f字段说明:v/编码格式/分辨率/帧率/码率类型/码率大小a/编码格式/码率大小/采样率

    int ret = eXosip_call_build_initial_invite(sip_context, &msg, to, from, nullptr, nullptr);
    if (ret) {
        LOG(ERROR) << fmt::format("eXosip_call_build_initial_invite error: {} {} ret:{}", from, to, ret);
        return -1;
    }

    osip_message_set_body(msg, sdp, strlen(sdp));
    osip_message_set_content_type(msg, "application/sdp");
    snprintf(session_exp, sizeof(session_exp) - 1, "%i;refresher=uac", s_info.timeout);
    osip_message_set_header(msg, "Session-Expires", session_exp);
    osip_message_set_supported(msg, "timer");

    int call_id = eXosip_call_send_initial_invite(sip_context, msg);

    if (call_id > 0) {
        LOG(INFO) << fmt::format("eXosip_call_send_initial_invite success: call_id={}", call_id);
    } else {
        LOG(ERROR) << fmt::format("eXosip_call_send_initial_invite error: call_id={}", call_id);
    }
    return ret;
}

int Handler::request_device_query(eXosip_t *sip_context, ClientRequestPtr req)
{
    ClientPtr client = req->client_ptr;
    if (!sip_context || !client) {
        return -1;
    }
    char str_from[512] = {0};
    char str_to[512] = {0};
    char str_body[2048] = {0};
    auto s_info = gServerInfo;
    sprintf(str_from, "sip:%s@%s:%d", s_info.sip_id.c_str(), s_info.ip.c_str(), s_info.port);
    sprintf(str_to, "sip:%s@%s:%d", client->device.c_str(), client->ip.c_str(), client->port);
    snprintf(str_body, 2048,
             "<?xml version=\"1.0\"?>"
             "<Query>"
             "<CmdType>Catalog</CmdType>"
             "<SN>%d</SN>"
             "<DeviceID>%s</DeviceID>"
             "</Query>",
             get_random_sn(), client->device.c_str());

    osip_message_t *message = nullptr;
    eXosip_message_build_request(sip_context, &message, "MESSAGE", str_to, str_from, nullptr);
    osip_message_set_body(message, str_body, strlen(str_body));
    osip_message_set_content_type(message, "Application/MANSCDP+xml");
    eXosip_lock(sip_context);
    int ret = eXosip_message_send_request(sip_context, message);
    LOG(INFO) << fmt::format("send device query ret:{}", ret);
    eXosip_unlock(sip_context);
    return 0;
}

int Handler::request_refresh_device_library(eXosip_t *sip_context, ClientRequestPtr req)
{
    ClientPtr client = req->client_ptr;
    if (!sip_context || !client) {
        return -1;
    }
    char str_from[512] = {0};
    char str_to[512] = {0};
    auto s_info = gServerInfo;
    sprintf(str_from, "sip:%s@%s:%d", s_info.sip_id.c_str(), s_info.ip.c_str(), s_info.port);
    sprintf(str_to, "sip:%s@%s:%d", client->real_device_id.c_str(), client->ip.c_str(), client->port);
    auto temp_ptr = std::make_shared<XmlQueryLibraryParam>();

    auto params_ptr = client->param_ptr;
    params_ptr->sn = get_random_sn();
    std::string str_body = MsgBuilder::instance()->BuildMsg(params_ptr);

    osip_message_t *message = nullptr;
    eXosip_message_build_request(sip_context, &message, "MESSAGE", str_to, str_from, nullptr);
    osip_message_set_body(message, str_body.c_str(), str_body.length());
    osip_message_set_content_type(message, "Application/MANSCDP+xml");
    eXosip_lock(sip_context);
    int ret = eXosip_message_send_request(sip_context, message);
    history_video_cache_[client->real_device_id] = 0;
    LOG(INFO) << fmt::format("send query device library ret:{}", ret);
    eXosip_unlock(sip_context);
    return 0;
}

int Handler::request_invite_playback(eXosip_t *sip_context, ClientRequestPtr req)
{
    char session_exp[1024] = {0};
    osip_message_t *msg = nullptr;
    char from[1024] = {0};
    char to[1024] = {0};
    char contact[1024] = {0};
    char sdp[2048] = {0};
    RequestParamQueryHistoryPtr param_ptr = std::dynamic_pointer_cast<RequestParamQueryHistory>(req->param_ptr);
    if (param_ptr == nullptr) {
        LOG(ERROR) << "param_ptr cast nullptr!";
        return -1;
    }
    ClientPtr client = req->client_ptr;
    auto s_info = gServerInfo;
    auto media_info = gMediaServerInfo;

    LOG(INFO) << fmt::format("addr:{}", client->rtsp_url.c_str());
    sprintf(from, "sip:%s@%s:%d", s_info.sip_id.c_str(), s_info.ip.c_str(), s_info.port);
    sprintf(contact, "sip:%s@%s:%d", s_info.sip_id.c_str(), s_info.ip.c_str(), s_info.port);
    sprintf(to, "sip:%s@%s:%d", client->real_device_id.c_str(), client->ip.c_str(), client->port);
    snprintf(sdp, 2048,
             "v=0\r\n"
             "o=%s 0 0 IN IP4 %s\r\n"
             "s=Playback\r\n"
             "u=%s:0\r\n"
             "c=IN IP4 %s\r\n"
             "t=%d %d\r\n"
             "m=video %d TCP/RTP/AVP 96 98 97\r\n"
             "a=recvonly\r\n"
             "a=rtpmap:96 PS/90000\r\n"
             "a=rtpmap:98 H264/90000\r\n"
             "a=rtpmap:97 MPEG4/90000\r\n"
             "a=downloadspeed:1\r\n"
             "a=setup:passive\r\n"
             "a=connection:new\r\n"
             "y=%s\r\n"
             "f=\r\n",
             client->real_device_id.c_str(), media_info.rtp_ip.c_str(), client->real_device_id.c_str(),
             media_info.rtp_ip.c_str(), param_ptr->start_time, param_ptr->end_time, media_info.rtp_port,
             client->ssrc.c_str());
    int ret = eXosip_call_build_initial_invite(sip_context, &msg, to, from, nullptr, nullptr);
    if (ret) {
        LOG(ERROR) << fmt::format("eXosip_call_build_initial_invite error: {} {} ret:{}", from, to, ret);
        return -1;
    }

    osip_message_set_body(msg, sdp, strlen(sdp));
    osip_message_set_content_type(msg, "application/sdp");
    snprintf(session_exp, sizeof(session_exp) - 1, "%i;refresher=uac", s_info.timeout);
    osip_message_set_header(msg, "Session-Expires", session_exp);
    osip_message_set_supported(msg, "timer");

    int call_id = eXosip_call_send_initial_invite(sip_context, msg);

    if (call_id > 0) {
        LOG(INFO) << fmt::format("eXosip_call_send_initial_invite success: call_id={}", call_id);
    } else {
        LOG(ERROR) << fmt::format("eXosip_call_send_initial_invite error: call_id={}", call_id);
    }
    return ret;
}

int Handler::request_broadcast(eXosip_t *sip_context, ClientRequestPtr req)
{
    ClientPtr client = req->client_ptr;
    if (!sip_context || !client) {
        LOG(ERROR) << "request_broadcast error, sip_context or client is nullptr!";
        return -1;
    }
    ClientInfoPtr client_info_ptr = nullptr;
    for (auto obj : client->client_infos_) {
        auto client_info = obj.second;
        if (client->device == client_info->parent_id) {  // 具有语音输出能力
            client_info_ptr = client_info;
            break;
        }
    }
    if (!client_info_ptr) {
        LOG(ERROR) << "can not find the device to output audio!";
        return -1;
    }
    char str_from[512] = {0};
    char str_to[512] = {0};
    char str_body[1024] = {0};
    generate_borad_cast_xml(str_from, str_to, str_body, client_info_ptr, client);
    osip_message_t *message = nullptr;
    eXosip_message_build_request(sip_context, &message, "MESSAGE", str_to, str_from, nullptr);
    osip_message_set_body(message, str_body, strlen(str_body));
    osip_message_set_content_type(message, "Application/MANSCDP+xml");
    eXosip_lock(sip_context);
    int ret = eXosip_message_send_request(sip_context, message);
    LOG(INFO) << fmt::format("send device query ret:{}", ret);
    eXosip_unlock(sip_context);
    return 0;
}

int Handler::request_fast_forward(eXosip_t *sip_context, ClientRequestPtr req)
{
    char session_exp[1024] = {0};
    osip_message_t *msg = nullptr;
    char from[1024] = {0};
    char to[1024] = {0};
    char contact[1024] = {0};
    char body_info[1024] = {0};
    ClientPtr client = req->client_ptr;
    auto s_info = gServerInfo;

    auto param_ptr = std::dynamic_pointer_cast<RequestParamFastforward>(req->param_ptr);
    if (!param_ptr) {
        LOG(ERROR) << "param_ptr is null pointer!!";
        return -1;
    }
    int did = Server::instance()->GetPlaybackId(param_ptr->ssrc);
    if (did < 0) {
        LOG(ERROR) << "can not find correct did!";
        return -1;
    }

    LOG(INFO) << fmt::format("addr:{}", client->rtsp_url.c_str());
    sprintf(from, "sip:%s@%s:%d", s_info.sip_id.c_str(), s_info.ip.c_str(), s_info.port);
    sprintf(contact, "sip:%s@%s:%d", s_info.sip_id.c_str(), s_info.ip.c_str(), s_info.port);
    sprintf(to, "sip:%s@%s:%d", client->real_device_id.c_str(), client->ip.c_str(), client->port);
    snprintf(body_info, 1024,
             "PLAY MANSTRSP/1.0\r\n"
             "CSeq:%d\r\n"
             "Scale:%s\r\n"
             "\r\n",
             3, param_ptr->scale.c_str());
    LOG(INFO) << fmt::format("request fast forward,did:{}", did);
    int ret = eXosip_call_build_info(sip_context, did, &msg);
    // int ret = eXosip_call_build_request(sip_context, did, "INFO", &msg);
    if (ret) {
        LOG(ERROR) << fmt::format("eXosip_call_build_request error: {} {} ret:{}", from, to, ret);
        return -1;
    }
    osip_message_set_body(msg, body_info, strlen(body_info));
    osip_message_set_content_type(msg, "Application/MANSRTSP");
    snprintf(session_exp, sizeof(session_exp) - 1, "%i;refresher=uac", s_info.timeout);
    int call_ret = eXosip_call_send_request(sip_context, did, msg);
    if (call_ret > 0) {
        LOG(INFO) << fmt::format("eXosip_message_send_request success: call_ret={}", call_ret);
    } else {
        LOG(ERROR) << fmt::format("eXosip_message_send_request error: call_ret={}", call_ret);
    }
#if 0
    int ret = eXosip_message_build_request(sip_context, &msg, "INFO", to, from, nullptr);
    if (ret) {
        LOGE( "eXosip_message_build_request error: %s %s ret:%d", from, to, ret);
        return -1;
    }
    
    osip_message_set_call_id(msg, param_ptr->call_id.c_str());
    osip_message_set_body(msg, body_info, strlen(body_info));
    osip_message_set_content_type(msg, "Application/MANSRTSP");
    snprintf(session_exp, sizeof(session_exp)-1, "%i;refresher=uac", s_info.timeout);
    int call_ret = eXosip_message_send_request(sip_context, msg);
    if (call_ret > 0) {
        LOGI("eXosip_message_send_request success: call_ret=%d",call_ret);
    }else{
        LOGE("eXosip_message_send_request error: call_ret=%d",call_ret);
    }
#endif
    return ret;
}

int Handler::request_rewind(eXosip_t *sip_context, ClientRequestPtr req) { return 0; }

int Handler::request_pasue(eXosip_t *sip_context, ClientRequestPtr req) { return 0; }

int Handler::parse_xml(const char *data, const char *s_mark, bool with_s_make, const char *e_mark, bool with_e_make,
                       char *dest)
{
    const char *satrt = strstr(data, s_mark);

    if (satrt != NULL) {
        const char *end = strstr(satrt, e_mark);

        if (end != NULL) {
            int s_pos = with_s_make ? 0 : strlen(s_mark);
            int e_pos = with_e_make ? strlen(e_mark) : 0;

            strncpy(dest, satrt + s_pos, (end + e_pos) - (satrt + s_pos));
        }
        return 0;
    }
    return -1;
}

/**
<Response>
<CmdType>Catalog</CmdType>
<SN>0</SN>
<DeviceID>34020000002000001001</DeviceID>
<SumNum>2</SumNum>
<DeviceList Num="2">
<Item>
<DeviceID>34020000001310000001</DeviceID>
<Name>200w</Name>
<Manufacturer>GBT28181</Manufacturer>
<Model>IP Camera</Model>
<Owner>Owner</Owner>
<CivilCode>3402000000</CivilCode>
<Address>Address</Address>
<Parental>0</Parental>
<ParentID>34020000001310000001</ParentID>
<SafetyWay>0</SafetyWay>
<RegisterWay>1</RegisterWay>
<Secrecy>0</Secrecy>
<Status>ON</Status>
</Item>
<Item>
<DeviceID>34020000001370000001</DeviceID>
<Name>AudioOut</Name>
<Manufacturer>GBT28181</Manufacturer>
<Model>AudioOut</Model>
<Owner>Owner</Owner>
<CivilCode>3402000000</CivilCode>
<Address>Address</Address>
<Parental>0</Parental>
<ParentID>34020000002000001001</ParentID>
<SafetyWay>0</SafetyWay>
<RegisterWay>1</RegisterWay>
<Secrecy>0</Secrecy>
<Status>ON</Status>
</Item>
</DeviceList>
</Response>
 */
int Handler::parse_device_xml(const std::string &xml_str)
{
    tinyxml2::XMLDocument doc;
    auto ret = doc.Parse(xml_str.c_str());
    if (ret != XMLError::XML_SUCCESS) {
        LOG(ERROR) << "parse device xml error!";
        return -1;
    }
    // 根元素
    XMLElement *root = doc.RootElement();
    // 指定名字的第一个子元素
    XMLElement *node_device_id = root->FirstChildElement("DeviceID");
    if (!node_device_id) {
        LOG(ERROR) << "parse device_id error!";
        return -2;
    }
    std::string device_id = node_device_id->GetText();
    XMLElement *node_device_list = root->FirstChildElement("DeviceList");
    if (!node_device_list) {
        LOG(ERROR) << "parse device list error!";
        return -3;
    }
    XMLElement *node_device_item = node_device_list->FirstChildElement("Item");
    int index = 0;
    std::string temp_str;
    std::unordered_map<std::string, ClientInfoPtr> client_infos;  // <device_id, client_info>
    XMLElement *temp_node = nullptr;
    const char *temp_text = nullptr;
    do {
        ClientInfoPtr client_info = std::make_shared<ClientInfo>();
        client_info->camera_manufacturer = kCameraManufacturerNone;
        XML_GET_STRING_DEFAULT(node_device_item, "DeviceID", client_info->device_id, temp_node, temp_text, device_id);
        XML_GET_STRING(node_device_item, "Name", client_info->name, temp_node, temp_text);
        XML_GET_STRING(node_device_item, "Manufacturer", client_info->manufacturer, temp_node, temp_text);
        XML_GET_STRING(node_device_item, "Model", client_info->model, temp_node, temp_text);
        XML_GET_STRING(node_device_item, "Owner", client_info->owner, temp_node, temp_text);
        XML_GET_STRING(node_device_item, "CivilCode", client_info->civil_code, temp_node, temp_text);
        XML_GET_STRING(node_device_item, "Address", client_info->address, temp_node, temp_text);
        XML_GET_INT(node_device_item, "Parental", client_info->parental, temp_node, temp_text);
        XML_GET_STRING(node_device_item, "ParentID", client_info->parent_id, temp_node, temp_text);
        XML_GET_INT(node_device_item, "SafetyWay", client_info->safety_way, temp_node, temp_text);
        XML_GET_INT(node_device_item, "RegisterWay", client_info->register_way, temp_node, temp_text);
        XML_GET_INT(node_device_item, "Secrecy", client_info->secrecy, temp_node, temp_text);
        temp_node = node_device_item->FirstChildElement("Status");
        if (temp_node) {
            temp_text = temp_node->GetText();
            if (temp_text) {
                client_info->status = (strcmp(temp_text, "ON") == 0) ? 1 : 0;
            }
        }
        std::string str_client_type = client_info->model;
        std::transform(str_client_type.begin(), str_client_type.end(), str_client_type.begin(),
                       [](unsigned char c) { return std::tolower(c); });
        std::string str_manufacturer = client_info->manufacturer;
        std::transform(str_manufacturer.begin(), str_manufacturer.end(), str_manufacturer.begin(),
                       [](unsigned char c) { return std::tolower(c); });
        if (str_manufacturer.find("hik") != std::string::npos) {
            client_info->camera_manufacturer = kCameraManufacturerHik;
        } else if (str_manufacturer.find("dahua") != std::string::npos) {
            client_info->camera_manufacturer = kCameraManufacturerDaHua;
        }
        auto s_info = gServerInfo;
        client_info->channel_type = kChannelNone;
        do {
            if (str_client_type.find("camera") != std::string::npos) {
                client_info->channel_type = kChannelVideo;
                break;
            }
            if (str_client_type.find("audio") != std::string::npos) {
                client_info->channel_type = kChannelAudio;
                break;
            }
            if (str_client_type.find("alarm") != std::string::npos) {
                client_info->channel_type = kChannelAlarm;
                break;
            }
            if (!boost::starts_with(client_info->device_id, s_info.realm)) {
                break;
            }
            auto id_type = client_info->device_id.substr(s_info.realm.length());
            if (id_type.length() < 3) {
                break;
            }
            // 根据id里13开头确定通道类型
            if (!id_type.compare(0, 3, "132")) {
                client_info->channel_type = kChannelVideo;
                break;
            }
            if (!id_type.compare(0, 3, "134")) {
                client_info->channel_type = kChannelAlarm;
                break;
            }
            if (!id_type.compare(0, 3, "137")) {
                client_info->channel_type = kChannelAudio;
                break;
            }
        } while (0);
        client_info->name = Xzm::util::Chinese::instance()->GBKToUTF8(client_info->name);
        client_infos[client_info->device_id] = client_info;
        node_device_item = node_device_item->NextSiblingElement("Item");
        LOG(INFO) << fmt::format(
            "index[{}]\n"
            "DeviceID    :{}\n"
            "Name        :{}\n"
            "Manufacturer:{}\n"
            "Model       :{}\n"
            "Owner       :{}\n"
            "CivilCode   :{}\n"
            "Address     :{}\n"
            "Parental    :{}\n"
            "ParentID    :{}\n"
            "RegisterWay :{}\n"
            "secrecy     :{}\n"
            "status:{}\n",
            index++, client_info->device_id, client_info->name, client_info->manufacturer, client_info->model,
            client_info->owner, client_info->civil_code, client_info->address, client_info->parental,
            client_info->parent_id, client_info->register_way, client_info->secrecy, client_info->status);
    } while (node_device_item);
    Server::instance()->UpdateClientInfo(device_id, client_infos);
    return 0;
}

int Handler::parse_alarm_xml(const std::string &xml_str)
{
    if (xml_str.empty()) {
        LOG(ERROR) << "报文为空";
        return -1;
    }
    tinyxml2::XMLDocument doc;
    auto ret = doc.Parse(xml_str.c_str());
    if (ret != XMLError::XML_SUCCESS) {
        LOG(ERROR) << "parse device xml error!";
        return -1;
    }
    // 根元素
    XMLElement *root = doc.RootElement();
    // 指定名字的第一个子元素
    XMLElement *node_alarm_info = root->FirstChildElement("Notify");
    if (!node_alarm_info) {
        LOG(ERROR) << "节点获取错误";
        return -2;
    }
    int index = 0;
    std::string temp_str;
    std::unordered_map<std::string, ClientInfoPtr> client_infos;  // <device_id, client_info>
    XMLElement *temp_node = nullptr;
    const char *temp_text = nullptr;
    std::string device_id, alarm_time;
    int alarm_priority = 0, alarm_method = 0;
    do {
        ClientInfoPtr client_info = std::make_shared<ClientInfo>();
        client_info->camera_manufacturer = kCameraManufacturerNone;
        XML_GET_STRING(node_alarm_info, "DeviceID", device_id, temp_node, temp_text);
        XML_GET_STRING(node_alarm_info, "AlarmTime", alarm_time, temp_node, temp_text);
        XML_GET_INT(node_alarm_info, "AlarmPriority", alarm_priority, temp_node, temp_text);
        XML_GET_INT(node_alarm_info, "AlarmMethod", alarm_method, temp_node, temp_text);
    } while (0);
    // TODO  告警信息后续如何处理
    return 0;
}

/*
<?xml version="1.0" encoding="gb2312"?>
<Response>
<CmdType>RecordInfo</CmdType>
<SN>18975</SN>
<DeviceID>34020000001320000005</DeviceID>
<Name>��ƺ�����Ӫҵ��</Name>
<SumNum>65</SumNum>
<RecordList Num="1">
<Item>
<DeviceID>34020000001320000005</DeviceID>
<Name>��ƺ�����Ӫҵ��</Name>
<FilePath>1695136627_1695139429</FilePath>
<Address>Address 1</Address>
<StartTime>2023-09-19T23:17:07</StartTime>
<EndTime>2023-09-19T23:59:59</EndTime>
<Secrecy>0</Secrecy>
<Type>time</Type>
</Item>
</RecordList>
</Response>
*/
int Handler::parse_recordinfo_xml(const std::string &xml_str, bool &is_last_item)
{
    do {
        // CLOGI(CYAN, "%s", xml_str.c_str());
        tinyxml2::XMLDocument doc;
        auto ret = doc.Parse(xml_str.c_str());
        if (ret != XMLError::XML_SUCCESS) {
            LOG(ERROR) << "parse recordinfo xml error!";
            break;
        }
        // 根元素
        XMLElement *root = doc.RootElement();
        std::string root_name = root->Name();
        if (root_name != "Response") {  // 只解析响应报文
            LOG(ERROR) << "parse response error!";
            return -1;
        }
        // 指定名字的第一个子元素
        XMLElement *node_device_id = root->FirstChildElement("DeviceID");
        if (!node_device_id) {
            LOG(ERROR) << "parse device_id error!";
            break;
        }
        std::string parent_device_id = node_device_id->GetText();
        // 指定名字的第一个子元素
        XMLElement *node_item_count = root->FirstChildElement("SumNum");
        if (!node_item_count) {
            LOG(ERROR) << "parse device_id error!";
            break;
        }
        std::string str_item_count = node_item_count->GetText();
        if (str_item_count.empty()) {
            str_item_count = "0";
        }
        int item_count = std::stoi(str_item_count);
        if (item_count <= 0) {  // 有些设备不存储历史录像
            break;
        }
        XMLElement *node_record_list = root->FirstChildElement("RecordList");
        if (!node_record_list) {
            LOG(ERROR) << "parse record list error!";
            break;
        }
        const XMLAttribute *attr = node_record_list->FindAttribute("Num");
        int item_num = attr->Int64Value();  // 本次xml有多少个记录

        XMLElement *node_record_item = node_record_list->FirstChildElement("Item");
        int index = 0;
        std::string temp_str;
        std::vector<RecordInfoPtr> record_infos;
        XMLElement *temp_node = nullptr;
        const char *temp_text = nullptr;
        do {
            RecordInfoPtr record_info = std::make_shared<RecordInfo>();
            record_info->current_num = item_num;
            record_info->device_id = node_record_item->FirstChildElement("DeviceID")->GetText();
            XML_GET_STRING(node_record_item, "Name", record_info->name, temp_node, temp_text);
            XML_GET_STRING(node_record_item, "FilePath", record_info->file_path, temp_node, temp_text);
            XML_GET_STRING(node_record_item, "Address", record_info->address, temp_node, temp_text);
            XML_GET_STRING(node_record_item, "StartTime", record_info->start_time, temp_node, temp_text);
            XML_GET_STRING(node_record_item, "end_time", record_info->end_time, temp_node, temp_text);
            XML_GET_STRING(node_record_item, "Type", record_info->type, temp_node, temp_text);
            XML_GET_INT(node_record_item, "Secrecy", record_info->secrecy, temp_node, temp_text);
            record_info->name = Xzm::util::Chinese::instance()->GBKToUTF8(record_info->name);
            record_infos.emplace_back(record_info);
            node_record_item = node_record_item->NextSiblingElement("Item");
            LOG(INFO) << fmt::format(
                "index[{}]:\n"
                "item_num    :{}\n"
                "current_num :{}\n"
                "item_count  :{}\n"
                "DeviceID    :{}\n"
                "Name        :{}\n"
                "FilePath    :{}\n"
                "Address     :{}\n"
                "StartTime   :{}\n"
                "EndTime     :{}\n"
                "Secrecy     :{}\n"
                "Type        :{}\n",
                index++, item_num, (int)history_video_cache_[parent_device_id], item_count, record_info->device_id,
                record_info->name, record_info->file_path, record_info->address, record_info->start_time,
                record_info->end_time, record_info->secrecy, record_info->type);
        } while (node_record_item);
        Server::instance()->AddRecordInfo(parent_device_id, record_infos);
        history_video_cache_[parent_device_id] += item_num;  // 视频的device_id和parane_device_id相同
        if (history_video_cache_[parent_device_id] >= item_count) {
            is_last_item = true;
        }
        return 0;
    } while (0);
    is_last_item = true;
    return -1;
}

void Handler::dump_request(eXosip_event_t *evtp)
{
    char *s;
    size_t len;
    if (evtp && evtp->request) {
        osip_message_to_str(evtp->request, &s, &len);
        LOG(INFO) << fmt::format(
            "\n********************print request "
            "start\ttype={}********************\n{}\n********************print "
            "request end********************\n",
            (int)evtp->type, s);
    }
}

void Handler::dump_response(eXosip_event_t *evtp)
{
    char *s;
    size_t len;
    if (evtp && evtp->response) {
        osip_message_to_str(evtp->response, &s, &len);
        LOG(INFO) << fmt::format(
            "\n********************print response "
            "start\ttype={}********************\n{}\n********************print "
            "response end********************\n",
            (int)evtp->type, s);
    }
}

int Handler::get_random_sn()
{
    std::default_random_engine e;
    std::uniform_int_distribution<int> u(9999, 100000);
    e.seed(time(0));
    return u(e);
}

int Handler::generate_borad_cast_xml(char *str_from, char *str_to, char *str_body, ClientInfoPtr client_info_ptr,
                                     ClientPtr client)
{
    auto s_info = gServerInfo;
    sprintf(str_from, "sip:%s@%s:%d", s_info.sip_id.c_str(), s_info.ip.c_str(), s_info.port);
    sprintf(str_to, "sip:%s@%s:%d", client->device.c_str(), client->ip.c_str(), client->port);
    switch (client_info_ptr->camera_manufacturer) {
        case kCameraManufacturerDaHua:
            snprintf(str_body, 1024,
                     "<?xml version=\"1.0\"?>"
                     "<Notify>"
                     "<CmdType>Broadcast</CmdType>"
                     "<SN>%d</SN>"
                     "<SourceID>%s</SourceID>"
                     "<TargetID>%s</TargetID>"
                     "</Notify>",
                     get_random_sn(), s_info.sip_id.c_str(), client_info_ptr->device_id.c_str());
            break;
        case kCameraManufacturerHik:
        default:
            snprintf(str_body, 1024,
                     "<?xml version=\"1.0\"?>"
                     "<Notify>"
                     "<CmdType>Broadcast</CmdType>"
                     "<SourceID>%s</SourceID>"
                     "<TargetID>%s</TargetID>"
                     "</Notify>",
                     s_info.sip_id.c_str(), client_info_ptr->device_id.c_str());
            break;
    }
    return 0;
}
};  // namespace Xzm
