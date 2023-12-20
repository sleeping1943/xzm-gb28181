#include "invite_handler.h"
#include "../utils/log.h"
#include <boost/algorithm/string.hpp>
#include <boost/regex.h>
#include <boost/algorithm/string/regex.hpp>
#include <cctype>
#include <chrono>
#include <iterator>
#include <memory>
#include <osipparser2/headers/osip_header.h>
#include <osipparser2/headers/osip_via.h>
#include <osipparser2/sdp_message.h>
#include <ostream>
#include "../server.h"
#include <hv/requests.h>
#include <stdlib.h>
#include <thread>
#include "../utils/helper.h"
#include "../utils/md5.h"

namespace Xzm
{
    InviteHandler::InviteHandler()
    {

    }

    InviteHandler::~InviteHandler()
    {

    }

    bool InviteHandler::Process(eXosip_event_t *evtp, eXosip_t* sip_context_, int code)
    {
        CLOGI(YELLOW, "invite handler Process!!");

        osip_body_t *body = nullptr;
        osip_message_get_body(evtp->request, 0, &body);
        std::string str_body(body->body);
        std::vector<std::string> str_vec;
        unsigned char md5_buf[16] = {0};
        std::string ssrc, client_ssrc;
        #if 1
        if (!str_body.empty()) {
            boost::split_regex(str_vec, str_body, boost::regex("\r\n"));
            for (auto& str : str_vec) {
                std::transform(str.begin(), str.end(), str.begin(), [] (int c) {
                    return std::tolower(c);
                });
                if (boost::starts_with(str, "y=")) {
                    ssrc = str.substr(2);
                    //ssrc = Xzm::util::convert10to16(ssrc);
                    //std::cout << "ssrc:" << ssrc << std::endl;
                } 
            }
            if (!ssrc.empty()) {
                //Server::instance()->AddPlacybackInfo(ssrc, evtp->did);
                CLOGI(YELLOW, "ssrc :%s", ssrc.c_str());
            }
        }
        #endif
        unsigned short talk_port = 0;
        sdp_message_t *sdp_msg = nullptr;
        std::string client_ip, str_port, username, session_id, session_ver, str_proto, media_type;
        ClientPtr client_ptr = nullptr;
        do {
            auto s_info = Server::instance()->GetServerInfo();
            sdp_message_init(&sdp_msg);
            sdp_message_parse(sdp_msg, body->body);
            client_ip = sdp_message_o_addr_get(sdp_msg);
            //client_ip = s_info.rtp_ip;
            str_port = sdp_message_m_port_get(sdp_msg, 0);
            username = sdp_message_o_username_get(sdp_msg);
            //username = s_info.sip_id;
            session_id = sdp_message_o_sess_id_get(sdp_msg);
            session_ver = sdp_message_o_sess_version_get(sdp_msg);
            str_proto = sdp_message_m_proto_get(sdp_msg, 0);
            media_type = sdp_message_m_media_get(sdp_msg, 0);

            printf("\nclient_ip:%s\tusername:%s\tsession_id:%s\tsession_ver:%s\tstr_port:%s\tstr_proto:%s\tmedia_type:%s\n" 
             ,client_ip.c_str(), username.c_str(), session_id.c_str(), session_ver.c_str(), str_port.c_str(), str_proto.c_str(), media_type.c_str());
            dump_request(evtp);
            dump_response(evtp);
            // 找到对应的client
            client_ptr = Server::instance()->FindClientEx(username);
            if (client_ptr == nullptr) {
                LOGE("can not find the client:%s", username.c_str());
                break;
            }
            if (client_ptr->is_talking) {
                client_ptr = nullptr;
                LOGE("client is talking");
                break;
            }
            /* 回复invite请求OK要携带SDP信息*/
            osip_message_t* msg = nullptr;
            //int ret = eXosip_call_build_ack(sip_context_, evtp->tid, &msg);   // 没有200状态码
            int ret = eXosip_call_build_answer(sip_context_, evtp->tid, 200, &msg);
            //int ret = eXosip_message_build_answer(sip_context_, evtp->tid, 200, &msg); // no call here
            if (ret != 0) {
                LOGE("InviteHandler:eXosip_call_build_ack failed!");
                break;
            }
            talk_port = Server::instance()->GetTalkPort();
            if (talk_port <= 0) {
                LOGE("can not get port for mediakit to speek");
                break;
            }
            auto md5_str = Xzm::MD5(username).toStr();
            std::transform(md5_str.begin(), md5_str.end(), md5_str.begin(), [](const char& c) {
                return std::toupper(c);
            });
            //ssrc = std::string(md5_buf);
            client_ssrc = md5_str.substr(md5_str.length() - 8);
            //int ret = eXosip_call_build_request(sip_context_, evtp->did, "INVITE", &msg);
            auto media_info = Server::instance()->GetMediaServerInfo();
            // 构建消息体
            char sdp[2048] = {0};
            {
                snprintf (sdp, 2048,
                "v=0\r\n"
                "o=%s 0 0 IN IP4 %s\r\n"
                "s=Play\r\n"
                "c=IN IP4 %s\r\n"
                "t=0 0\r\n"
                "m=%s %d %s 8 96\r\n"
                "a=sendonly\r\n"
                "a=rtpmap:8 PCMA/8000\r\n"
                //"a=rtpmap:96 PS/90000\r\n"
                //"a=setup:passive\r\n"
                //"a=connection:new\r\n"
                "y=%s\r\n"
                "f=v/////a/1/8/1\r\n", s_info.sip_id.c_str(), media_info.rtp_ip.c_str(), media_info.rtp_ip.c_str(),
                media_type.c_str(), talk_port, str_proto.c_str(), ssrc.c_str());
                //"f=\r\n", username.c_str(),client_ip.c_str(), client_ip.c_str(), str_port.c_str(), str_proto.c_str(), ssrc.c_str());
            }
            osip_message_set_body(msg, sdp, strlen(sdp));
            osip_message_set_content_type(msg, "application/sdp");
            ret = eXosip_call_send_answer(sip_context_, evtp->tid, 200, msg);
            //ret = eXosip_message_send_answer(sip_context_, evtp->tid, 200, msg);
            //ret = eXosip_call_send_ack(sip_context_, evtp->tid, msg);
            //ret = eXosip_call_send_request(sip_context_, evtp->did, msg);
            if (ret == 0) {
                LOGI("InviteHandler:eXosip_call_send_answer OK");
            } else {
                LOGE("InviteHandler:eXosip_call_send_answer error=%d", ret);
            }
        } while(0);
        if (sdp_msg) {
            sdp_message_free(sdp_msg);
            sdp_msg = nullptr;
        }
        if (client_ptr == nullptr) {
            LOGE("can not find the client[%s] or client is talking!", username.c_str());
            return false;
        }
        // 发送openRtpServer的http请求,确定发送端口
        int ser_port = 0, ret_code = 0;
        std::string secret = "Lsb4XJqAdK0QLVErbKEvBBGrSDJ3lexS";
        std::string stream_id = TALK_PREFIX;
        std::string str_app = "rtp";
        stream_id += client_ssrc;
        // 以下推流到摄像头逻辑修改为推流后执行
    #if 1
        int is_udp = 1, pt=8, use_ps=1, only_audio=1;
        char publish_url[1024] = {0};
        snprintf(publish_url, 1024,
         "http://%s/index/api/startSendRtp?"
         "secret=%s&vhost=__defaultVhost__&app=rtp&stream=%s&src_port=%d&ssrc=%s&dst_url=%s&dst_port=%s&is_udp=%d&pt=%d&use_ps=%d&only_audio=%d",
         Server::instance()->GetMediaServerInfo().rtp_ip.c_str(),"Lsb4XJqAdK0QLVErbKEvBBGrSDJ3lexS"
         //, str_app.c_str(), stream_id.c_str(), stream_id.c_str(),"10.23.132.77","8020",/*client_ip.c_str(),str_port.c_str(),* / is_udp, pt, use_ps, only_audio
         , stream_id.c_str(), talk_port, stream_id.c_str(),client_ip.c_str(),str_port.c_str(), is_udp, pt, use_ps, only_audio
         );
        CLOGI(YELLOW, "url:%s", publish_url);
        auto resp = requests::get(publish_url);
        auto ret_json = resp->GetJson();
        int publish_code = 0;
        HV_JSON_GET_INT(ret_json, publish_code, "code");
        if (publish_code != 0) {
            LOGE("startSendRtp failed!,code:%d", publish_code);
            return false;
        }
        std::cout << "\n----------------------------body:" << resp->body << std::endl;
    #endif
        return true;
    }
};