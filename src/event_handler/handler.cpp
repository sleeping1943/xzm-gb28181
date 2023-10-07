#include "handler.h"
#include "../utils/log.h"
#include <cctype>
#include <chrono>
#include <memory>
#include <osipparser2/headers/osip_header.h>
#include <osipparser2/osip_message.h>
#include <osipparser2/osip_parser.h>
#include <ostream>
#include <random>
#include <string.h>
#include "../server.h"
#include "../utils/helper.h"
#include "../utils/tinyxml2.h"
#include "../msg_builder/msg_builder.h"
#include <algorithm>
#include "../utils/chinese.h"

using tinyxml2::XMLDocument;
using tinyxml2::XMLError;
using tinyxml2::XMLElement;
using tinyxml2::XMLAttribute;

using std::chrono::seconds;
using std::chrono::duration_cast;
using std::chrono::system_clock;

namespace Xzm {

uint64_t Handler::sn_ = 10000;

std::unordered_map<std::string, FUNC_MSG_RESPONSE> msg_response_;

Handler::Handler()
{

}

Handler::~Handler()
{

}

bool Handler::Process(eXosip_event_t *evtp, eXosip_t* sip_context_, int code)
{
    is_print = true;
    std::cout << "Handler Process!!" << std::endl;
    this->response_message(evtp, sip_context_, code);
    if (is_print) {
        this->dump_request(evtp);
        this->dump_response(evtp);
    }
    return true;
}

int Handler::request_bye(eXosip_event_t *evtp, eXosip_t *sip_context_)
{
    eXosip_lock(sip_context_);
    int ret = eXosip_call_terminate(sip_context_, evtp->cid, evtp->did);
    eXosip_unlock(sip_context_);
    return 0;
}

void Handler::response_message(eXosip_event_t *evtp, eXosip_t * sip_context_, int code)
{
    if (evtp == nullptr || evtp->request == nullptr) {
        LOGE("evtp or evtp->requets is nullptr!");
        return;
    }

    auto cseq_t = osip_message_get_cseq(evtp->request);
    std::string method = cseq_t ? cseq_t->method : "";
    std::transform(method.begin(), method.end(), method.begin(), [] (char c) {
        return std::tolower(c);
    });
    if (method == "ack"
    || method == "invite") {  // invite/ack等包没有xml报文,不能解析出cmdtype和deviceid,不响应即可
        return;
    }
    osip_body_t* body = nullptr;
    char CmdType[64] = {0};
    char DeviceID[64] = {0};
    // 获取sip协议中message消息body体xml数据并解析
    osip_message_get_body(evtp->request, 0, &body);
    if(body){
        parse_xml(body->body, "<CmdType>", false, "</CmdType>", false, CmdType);
        parse_xml(body->body, "<DeviceID>", false, "</DeviceID>", false, DeviceID);
        //CLOGI(YELLOW, "%s", body->body);
    }

    if (Server::is_server_quit) {    // 已经开始关闭服务,删除该客户端,发送bye
        Server::instance()->RemoveClient(DeviceID);
        request_bye(evtp, sip_context_);
        return;
    }
    if (!Server::instance()->IsClientExist(DeviceID)
    && !Server::instance()->IsClientInfoExist(DeviceID)) {  // 服务器没有此客户端信息,也不是音频通道ID,断开连接
        request_bye(evtp, sip_context_);
        return;
    }

    LOGI("CmdType=%s,DeviceID=%s", CmdType,DeviceID);
    auto func = Server::instance()->GetMsgResponse(CmdType);
    std::shared_ptr<boost::any> param_ptr = std::make_shared<boost::any>();
    if (func) {
        func(evtp, sip_context_, 200, nullptr);
    } else {
        this->response_message_answer(evtp, sip_context_, 200); // 默认处理
    }
    //if(!strcmp(CmdType, "Catalog")) {
    //    this->parse_device_xml(body->body);
    //    this->response_message_answer(evtp, sip_context_, 200);
    //    // 需要根据对方的Catelog请求，做一些相应的应答请求
    //} else if(!strcmp(CmdType, "Keepalive")){   // 心跳消息
    //    is_print = false;
    //    this->response_message_answer(evtp, sip_context_, 200);
    //}else{
    //    this->response_message_answer(evtp, sip_context_, 200);
    //}
    return;
}

void Handler::response_message_answer(eXosip_event_t *evtp, eXosip_t * sip_context_, int code)
{
    int returnCode = 0 ;
    osip_message_t * pRegister = nullptr;
    returnCode = eXosip_message_build_answer (sip_context_,evtp->tid,code,&pRegister);
    bool bRegister = false;
    if(pRegister){
        bRegister = true;
    }
    if (returnCode == 0 && bRegister)
    {
        eXosip_lock(sip_context_);
        eXosip_message_send_answer (sip_context_,evtp->tid,code,pRegister);
        eXosip_unlock(sip_context_);
    }
    else{
        LOGE("code=%d,returnCode=%d,bRegister=%d",code,returnCode,bRegister);
    }

}

void Handler::response_catalog(eXosip_event_t *evtp, eXosip_t * sip_context_, int code, std::shared_ptr<boost::any> param)
{
    osip_body_t* body = nullptr;
    osip_message_get_body(evtp->request, 0, &body);
    parse_device_xml(body->body);
    response_message_answer(evtp, sip_context_, 200);
    return;
}

void Handler::response_recordinfo(eXosip_event_t *evtp, eXosip_t * sip_context_, int code, std::shared_ptr<boost::any> param)
{
    osip_body_t* body = nullptr;
    osip_message_get_body(evtp->request, 0, &body);
    bool is_last_item = false;
    parse_recordinfo_xml(body->body, is_last_item);
    if (is_last_item) { // 该设备历史录像获取完毕
        // 通知获取完成,返回前端录像信息
        Server::instance()->NotifyHistoryComplete();
        CLOGE(RED, "query history completed!!Notify histtory complete!");
    }
    response_message_answer(evtp, sip_context_, 200);
    return;
}

void Handler::response_keepalive(eXosip_event_t *evtp, eXosip_t * sip_context_, int code, std::shared_ptr<boost::any> param)
{
    is_print = false;
    this->response_message_answer(evtp, sip_context_, 200);
}

int Handler::request_invite(eXosip_t *sip_context, ClientRequestPtr req)
{
    char session_exp[1024] = { 0 };
    osip_message_t *msg = nullptr;
    char from[1024] = {0};
    char to[1024] = {0};
    char contact[1024] = {0};
    char sdp[2048] = {0};
    char head[1024] = {0};
    ClientPtr  client = req->client_ptr;
    /*
        在http请求推流时就确定rtsp推流地址
    */
    auto s_info = Server::instance()->GetServerInfo();
    //client->ssrc = Xzm::util::build_ssrc(true, s_info.realm);
    //auto ssrc = Xzm::util::convert10to16(client->ssrc);
    //client->rtsp_url = Xzm::util::get_rtsp_addr(s_info.rtp_ip, ssrc);
    
    CLOGI(RED, "addr:%s", client->rtsp_url.c_str());
    sprintf(from, "sip:%s@%s:%d", s_info.sip_id.c_str(),s_info.ip.c_str(), s_info.port);
    sprintf(contact, "sip:%s@%s:%d", s_info.sip_id.c_str(),s_info.ip.c_str(), s_info.port);
    sprintf(to, "sip:%s@%s:%d", client->real_device_id.c_str(), client->ip.c_str(), client->port);
    snprintf (sdp, 2048,
              "v=0\r\n"
              "o=%s 0 0 IN IP4 %s\r\n"
              "s=Play\r\n"
              "c=IN IP4 %s\r\n"
              "t=0 0\r\n"
              "m=video %d TCP/RTP/AVP 96 98 97\r\n"
              "a=recvonly\r\n"
              "a=rtpmap:96 PS/90000\r\n"
              "a=rtpmap:98 H264/90000\r\n"
              "a=rtpmap:97 MPEG4/90000\r\n"
              "a=setup:passive\r\n"
              "a=connection:new\r\n"
              "y=%s\r\n"
              "f=\r\n", client->real_device_id.c_str(),s_info.rtp_ip.c_str(), s_info.rtp_ip.c_str(), s_info.rtp_port, client->ssrc.c_str());
              //"y=0100000001\r\n"
              //"f=\r\n", s_info.sip_id.c_str(),s_info.ip.c_str(), s_info.rtp_ip.c_str(), s_info.rtp_port);

    int ret = eXosip_call_build_initial_invite(sip_context, &msg, to, from,  nullptr, nullptr);
    if (ret) {
        LOGE( "eXosip_call_build_initial_invite error: %s %s ret:%d", from, to, ret);
        return -1;
    }

    osip_message_set_body(msg, sdp, strlen(sdp));
    osip_message_set_content_type(msg, "application/sdp");
    snprintf(session_exp, sizeof(session_exp)-1, "%i;refresher=uac", s_info.timeout);
    osip_message_set_header(msg, "Session-Expires", session_exp);
    osip_message_set_supported(msg, "timer");

    int call_id = eXosip_call_send_initial_invite(sip_context, msg);

    if (call_id > 0) {
        LOGI("eXosip_call_send_initial_invite success: call_id=%d",call_id);
    }else{
        LOGE("eXosip_call_send_initial_invite error: call_id=%d",call_id);
    }
    return ret;
}

int Handler::request_invite_talk(eXosip_t *sip_context, ClientRequestPtr req)
{
    char session_exp[1024] = { 0 };
    osip_message_t *msg = nullptr;
    char from[1024] = {0};
    char to[1024] = {0};
    char contact[1024] = {0};
    char sdp[2048] = {0};
    char head[1024] = {0};
    ClientPtr client = req->client_ptr;

    auto s_info = Server::instance()->GetServerInfo();
    client->ssrc = Xzm::util::build_ssrc(true, s_info.realm);
    auto ssrc = Xzm::util::convert10to16(client->ssrc);
    client->rtsp_url = Xzm::util::get_rtsp_addr(s_info.rtp_ip, ssrc);
    
    CLOGI(RED, "addr:%s", client->rtsp_url.c_str());
    sprintf(from, "sip:%s@%s:%d", s_info.sip_id.c_str(),s_info.ip.c_str(), s_info.port);
    sprintf(contact, "sip:%s@%s:%d", s_info.sip_id.c_str(),s_info.ip.c_str(), s_info.port);
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
              "f=v/a/1/8/1=\r\n", client->device.c_str(),s_info.rtp_ip.c_str(), s_info.rtp_ip.c_str(), s_info.rtp_port, client->ssrc.c_str());
              //"y=0100000001\r\n"
              //"f=\r\n", s_info.sip_id.c_str(),s_info.ip.c_str(), s_info.rtp_ip.c_str(), s_info.rtp_port);
    */
    snprintf (sdp, 2048,
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
              "f=v/////a/1/8/1\r\n", client->device.c_str(),s_info.rtp_ip.c_str(), s_info.rtp_ip.c_str(), s_info.rtp_port, client->ssrc.c_str());
              //"y=0100000001\r\n"
              //"f=\r\n", s_info.sip_id.c_str(),s_info.ip.c_str(), s_info.rtp_ip.c_str(), s_info.rtp_port);
             // f字段说明:v/编码格式/分辨率/帧率/码率类型/码率大小a/编码格式/码率大小/采样率

    int ret = eXosip_call_build_initial_invite(sip_context, &msg, to, from,  nullptr, nullptr);
    if (ret) {
        LOGE( "eXosip_call_build_initial_invite error: %s %s ret:%d", from, to, ret);
        return -1;
    }

    osip_message_set_body(msg, sdp, strlen(sdp));
    osip_message_set_content_type(msg, "application/sdp");
    snprintf(session_exp, sizeof(session_exp)-1, "%i;refresher=uac", s_info.timeout);
    osip_message_set_header(msg, "Session-Expires", session_exp);
    osip_message_set_supported(msg, "timer");

    int call_id = eXosip_call_send_initial_invite(sip_context, msg);

    if (call_id > 0) {
        LOGI("eXosip_call_send_initial_invite success: call_id=%d",call_id);
    }else{
        LOGE("eXosip_call_send_initial_invite error: call_id=%d",call_id);
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
    auto s_info = Server::instance()->GetServerInfo();
    sprintf(str_from, "sip:%s@%s:%d", s_info.sip_id.c_str(), s_info.ip.c_str(), s_info.port);
    sprintf(str_to, "sip:%s@%s:%d", client->device.c_str(), client->ip.c_str(), client->port);
    snprintf(str_body, 2048,
    "<?xml version=\"1.0\"?>"\
    "<Query>"   \
    "<CmdType>Catalog</CmdType>"    \
    "<SN>%d</SN>"  \
    "<DeviceID>%s</DeviceID>" \
    "</Query>", get_random_sn(), client->device.c_str()
    );

    osip_message_t *message = nullptr;
    eXosip_message_build_request(sip_context, &message, "MESSAGE", str_to, str_from, nullptr);
    osip_message_set_body(message, str_body, strlen(str_body));
    osip_message_set_content_type(message, "Application/MANSCDP+xml");
    eXosip_lock(sip_context);
    int ret = eXosip_message_send_request(sip_context, message);
    CLOGI(RED, "send device query ret:%d", ret);
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
    auto s_info = Server::instance()->GetServerInfo();
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
    CLOGI(RED, "send query device library ret:%d", ret);
    eXosip_unlock(sip_context);
    return 0;
}

int Handler::request_invite_playback(eXosip_t *sip_context, ClientRequestPtr req)
{
    char session_exp[1024] = { 0 };
    osip_message_t *msg = nullptr;
    char from[1024] = {0};
    char to[1024] = {0};
    char contact[1024] = {0};
    char sdp[2048] = {0};
    char head[1024] = {0};
    char start_time[32] = {0};
    char end_time[32] = {0};
    RequestParamQueryHistoryPtr param_ptr = std::dynamic_pointer_cast<RequestParamQueryHistory>(req->param_ptr);
    if (param_ptr == nullptr) {
        CLOGE(RED, "param_ptr cast nullptr!");
        return -1;
    }
    ClientPtr client = req->client_ptr;
    auto s_info = Server::instance()->GetServerInfo();

    CLOGI(RED, "addr:%s", client->rtsp_url.c_str());
    sprintf(from, "sip:%s@%s:%d", s_info.sip_id.c_str(),s_info.ip.c_str(), s_info.port);
    sprintf(contact, "sip:%s@%s:%d", s_info.sip_id.c_str(),s_info.ip.c_str(), s_info.port);
    sprintf(to, "sip:%s@%s:%d", client->real_device_id.c_str(), client->ip.c_str(), client->port);
    snprintf (sdp, 2048,
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
              "f=\r\n"
              , client->real_device_id.c_str()
              , s_info.rtp_ip.c_str()
              , client->real_device_id.c_str()
              , s_info.rtp_ip.c_str()
              , param_ptr->start_time
              , param_ptr->end_time
              , s_info.rtp_port
              , client->ssrc.c_str());
    int ret = eXosip_call_build_initial_invite(sip_context, &msg, to, from,  nullptr, nullptr);
    if (ret) {
        LOGE( "eXosip_call_build_initial_invite error: %s %s ret:%d", from, to, ret);
        return -1;
    }

    osip_message_set_body(msg, sdp, strlen(sdp));
    osip_message_set_content_type(msg, "application/sdp");
    snprintf(session_exp, sizeof(session_exp)-1, "%i;refresher=uac", s_info.timeout);
    osip_message_set_header(msg, "Session-Expires", session_exp);
    osip_message_set_supported(msg, "timer");

    int call_id = eXosip_call_send_initial_invite(sip_context, msg);

    if (call_id > 0) {
        LOGI("eXosip_call_send_initial_invite success: call_id=%d",call_id);
    }else{
        LOGE("eXosip_call_send_initial_invite error: call_id=%d",call_id);
    }
    return ret;
}

int Handler::request_broadcast(eXosip_t *sip_context, ClientRequestPtr req)
{
    ClientPtr client = req->client_ptr;
    if (!sip_context || !client) {
        LOGE("request_broadcast error, sip_context or client is nullptr!");
        return -1;
    }
    ClientInfoPtr client_info_ptr = nullptr;
    for (auto obj : client->client_infos_) {
        auto client_info = obj.second;
        if (client->device == client_info->parent_id) { // 具有语音输出能力
            client_info_ptr = client_info;
            break;
        }
    }
    if (!client_info_ptr) {
        LOGE("can not find the device to output audio!");
        return -1;
    }
    char str_from[512] = {0};
    char str_to[512] = {0};
    char str_body[1024] = {0};
    auto s_info = Server::instance()->GetServerInfo();
    sprintf(str_from, "sip:%s@%s:%d", s_info.sip_id.c_str(), s_info.ip.c_str(), s_info.port);
    sprintf(str_to, "sip:%s@%s:%d", client->device.c_str(), client->ip.c_str(), client->port);
    snprintf(str_body, 1024,
    "<?xml version=\"1.0\"?>"\
    "<Notify>"   \
    "<CmdType>Broadcast</CmdType>"    \
    /*"<SN>248</SN>"  \*/
    "<SourceID>%s</SourceID>" \
    "<TargetID>%s</TargetID>" \
    "</Notify>", s_info.sip_id.c_str(), client_info_ptr->device_id.c_str()
    );

    osip_message_t *message = nullptr;
    eXosip_message_build_request(sip_context, &message, "MESSAGE", str_to, str_from, nullptr);
    osip_message_set_body(message, str_body, strlen(str_body));
    osip_message_set_content_type(message, "Application/MANSCDP+xml");
    eXosip_lock(sip_context);
    int ret = eXosip_message_send_request(sip_context, message);
    CLOGI(RED, "send device query ret:%d", ret);
    eXosip_unlock(sip_context);
    return 0;
}

int Handler::request_fast_forward(eXosip_t *sip_context, ClientRequestPtr req)
{
    char session_exp[1024] = { 0 };
    osip_message_t *msg = nullptr;
    char from[1024] = {0};
    char to[1024] = {0};
    char contact[1024] = {0};
    char body_info[1024] = {0};
    ClientPtr  client = req->client_ptr;
    auto s_info = Server::instance()->GetServerInfo();

    auto param_ptr = std::dynamic_pointer_cast<RequestParamFastforward>(req->param_ptr);
    if (!param_ptr) {
        CLOGE(RED, "param_ptr is null pointer!!");
        return -1;
    }
    int did = Server::instance()->GetPlaybackId(param_ptr->ssrc);
    if (did < 0) {
        CLOGE(RED, "can not find correct did!");
        return -1;
    }
    
    CLOGI(RED, "addr:%s", client->rtsp_url.c_str());
    sprintf(from, "sip:%s@%s:%d", s_info.sip_id.c_str(),s_info.ip.c_str(), s_info.port);
    sprintf(contact, "sip:%s@%s:%d", s_info.sip_id.c_str(),s_info.ip.c_str(), s_info.port);
    sprintf(to, "sip:%s@%s:%d", client->real_device_id.c_str(), client->ip.c_str(), client->port);
    snprintf (body_info, 1024,
              "PLAY MANSTRSP/1.0\r\n"
              "CSeq:%d\r\n"
              "Scale:%s\r\n"
              "\r\n", 3, param_ptr->scale.c_str());
    CLOGI(RED, "request fast forward,did:%d", did);
    int ret = eXosip_call_build_info(sip_context, did, &msg);
    //int ret = eXosip_call_build_request(sip_context, did, "INFO", &msg);
    if (ret) {
        LOGE( "eXosip_call_build_request error: %s %s ret:%d", from, to, ret);
        return -1;
    }
    osip_message_set_body(msg, body_info, strlen(body_info));
    osip_message_set_content_type(msg, "Application/MANSRTSP");
    snprintf(session_exp, sizeof(session_exp)-1, "%i;refresher=uac", s_info.timeout);
    int call_ret = eXosip_call_send_request(sip_context, did, msg);
    if (call_ret > 0) {
        LOGI("eXosip_message_send_request success: call_ret=%d",call_ret);
    }else{
        LOGE("eXosip_message_send_request error: call_ret=%d",call_ret);
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

int Handler::request_rewind(eXosip_t *sip_context, ClientRequestPtr req)
{
    return 0;
}

int Handler::request_pasue(eXosip_t *sip_context, ClientRequestPtr req)
{
    return 0;
}

int Handler::parse_xml(const char *data, const char *s_mark, bool with_s_make, const char *e_mark, bool with_e_make, char *dest)
{
    const char* satrt = strstr( data, s_mark );

    if(satrt != NULL) {
        const char* end = strstr(satrt, e_mark);

        if(end != NULL){
            int s_pos = with_s_make ? 0 : strlen(s_mark);
            int e_pos = with_e_make ? strlen(e_mark) : 0;

            strncpy( dest, satrt+s_pos, (end+e_pos) - (satrt+s_pos) );
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
int Handler::parse_device_xml(const std::string& xml_str)
{
    XMLDocument doc;
    auto ret = doc.Parse(xml_str.c_str());
    if (ret != XMLError::XML_SUCCESS) {
        LOGE("parse device xml error!");
        return -1;
    }
    // 根元素
    XMLElement *root = doc.RootElement();
    // 指定名字的第一个子元素
    XMLElement *node_device_id = root->FirstChildElement("DeviceID");
    if (!node_device_id) {
        LOGE("parse device_id error!");
        return -2;
    }
    std::string device_id = node_device_id->GetText();
    XMLElement *node_device_list = root->FirstChildElement("DeviceList");
    if (!node_device_list) {
        LOGE("parse device list error!");
        return -3;
    }
    XMLElement *node_device_item = node_device_list->FirstChildElement("Item");
    int index = 0;
    std::string temp_str;
    std::stringstream ss;
    std::unordered_map<std::string, ClientInfoPtr> client_infos;   // <device_id, client_info>
    XMLElement *temp_node = nullptr;
    const char* temp_text = nullptr;
    do {
        ClientInfoPtr client_info = std::make_shared<ClientInfo>();
        client_info->device_id = node_device_item->FirstChildElement("DeviceID")->GetText();
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
        std::transform(str_client_type.begin(), str_client_type.end(), str_client_type.begin()
        , [] (unsigned char c) {
                return std::tolower(c);
            });
        if (str_client_type.find("camera") != std::string::npos) {
            client_info->channel_type = kChannelVideo;
        } else if (str_client_type.find("audio") != std::string::npos) {
            client_info->channel_type = kChannelAudio;
        } else if (str_client_type.find("alarm") != std::string::npos) {
            client_info->channel_type = kChannelAlarm;
        } else {
            client_info->channel_type = kChannelNone;
        }
        client_info->name = Xzm::util::Chinese::instance()->GBKToUTF8(client_info->name);
        client_infos[client_info->device_id] = client_info;
        node_device_item = node_device_item->NextSiblingElement("Item");
        ss << "index[" << index++ << "]:" << std::endl
        << "DeviceID    :" << client_info->device_id << std::endl
        << "Name        :" << client_info->name << std::endl
        << "Manufacturer:" << client_info->manufacturer << std::endl
        << "Model       :" << client_info->model << std::endl
        << "Owner       :" << client_info->owner << std::endl
        << "CivilCode   :" << client_info->civil_code << std::endl
        << "Address     :" << client_info->address << std::endl
        << "Parental    :" << client_info->parental << std::endl
        << "ParentID    :" << client_info->parent_id << std::endl
        << "RegisterWay :" << client_info->register_way << std::endl
        << "secrecy     :" << client_info->secrecy << std::endl
        << "status:" << client_info->status << std::endl;
        CLOGI(RED, "%s", ss.str().c_str());
        ss.str("");
    } while (node_device_item);
    Server::instance()->UpdateClientInfo(device_id, client_infos);
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
int Handler::parse_recordinfo_xml(const std::string& xml_str, bool& is_last_item)
{
    do {
        //CLOGI(CYAN, "%s", xml_str.c_str());
        XMLDocument doc;
        auto ret = doc.Parse(xml_str.c_str());
        if (ret != XMLError::XML_SUCCESS) {
            LOGE("parse recordinfo xml error!");
            break;
        }
        // 根元素
        XMLElement *root = doc.RootElement();
        std::string root_name = root->Name();
        if (root_name != "Response") {  // 只解析响应报文
            LOGE("parse response error!");
            return -1;
        }
        // 指定名字的第一个子元素
        XMLElement *node_device_id = root->FirstChildElement("DeviceID");
        if (!node_device_id) {
            LOGE("parse device_id error!");
            break;
        }
        std::string parent_device_id = node_device_id->GetText();
        // 指定名字的第一个子元素
        XMLElement *node_item_count = root->FirstChildElement("SumNum");
        if (!node_item_count) {
            LOGE("parse device_id error!");
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
            LOGE("parse record list error!");
            break;
        }
        const XMLAttribute* attr = node_record_list->FindAttribute("Num");
        int item_num = attr->Int64Value();   // 本次xml有多少个记录
        

        XMLElement *node_record_item = node_record_list->FirstChildElement("Item");
        int index = 0;
        std::string temp_str;
        std::stringstream ss;
        std::vector<RecordInfoPtr> record_infos;
        XMLElement *temp_node = nullptr;
        const char* temp_text = nullptr;
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
            ss << "index[" << index++ << "]:" << std::endl
            << "item_num :" << item_num << std::endl
            << "current_num :" << history_video_cache_[parent_device_id] << std::endl
            << "item_count  :" << item_count << std::endl
            << "DeviceID    :" << record_info->device_id << std::endl
            << "Name        :" << record_info->name << std::endl
            << "FilePath:" << record_info->file_path << std::endl
            << "Address     :" << record_info->address << std::endl
            << "StartTime   :" << record_info->start_time << std::endl
            << "EndTime     :" << record_info->end_time << std::endl
            << "Secrecy     :" << record_info->secrecy << std::endl
            << "Type        :" << record_info->type << std::endl;
            CLOGI(RED, "%s", ss.str().c_str());
            ss.str("");
        } while (node_record_item);
        Server::instance()->AddRecordInfo(parent_device_id, record_infos);
        history_video_cache_[parent_device_id] += item_num;   // 视频的device_id和parane_device_id相同
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
    osip_message_to_str(evtp->request, &s, &len);
    CLOGI(YELLOW, "\n********************print request start\ttype=%d********************\n%s\n********************print request end********************\n",evtp->type,s);
}

void Handler::dump_response(eXosip_event_t *evtp)
{
    char *s;
    size_t len;
    osip_message_to_str(evtp->response, &s, &len);
    CLOGI(BLUE, "\n********************print response start\ttype=%d********************\n%s\n********************print response end********************\n",evtp->type,s);
}

int Handler::get_random_sn()
{
    std::default_random_engine e;
    std::uniform_int_distribution<int> u(9999, 100000);
    e.seed(time(0));
    return u(e);
}
};
