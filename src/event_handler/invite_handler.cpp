#include "invite_handler.h"
#include "../utils/log.h"
#include <boost/algorithm/string.hpp>
#include <boost/regex.h>
#include <boost/algorithm/string/regex.hpp>
#include <chrono>
#include <osipparser2/headers/osip_via.h>
#include <osipparser2/sdp_message.h>
#include <ostream>
#include "../server.h"
#include <hv/requests.h>
#include <stdlib.h>
#include <thread>
#include "../utils/helper.h"

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
        std::string ssrc;
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

        sdp_message_t *sdp_msg = nullptr;
        std::string client_ip, str_port, username, session_id, session_ver, str_proto, media_type;
        ClientPtr client_ptr = nullptr;
        do {
            sdp_message_init(&sdp_msg);
            sdp_message_parse(sdp_msg, body->body);
            client_ip = sdp_message_o_addr_get(sdp_msg);
            str_port = sdp_message_m_port_get(sdp_msg, 0);
            username = sdp_message_o_username_get(sdp_msg);
            session_id = sdp_message_o_sess_id_get(sdp_msg);
            session_ver = sdp_message_o_sess_version_get(sdp_msg);
            str_proto = sdp_message_m_proto_get(sdp_msg, 0);
            media_type = sdp_message_m_media_get(sdp_msg, 0);

            printf("\nclient_ip:%s\tusername:%s\tsession_id:%s\tsession_ver:%s\tstr_port:%s\tstr_proto:%s\tmedia_type:%s\n" 
             ,client_ip.c_str(), username.c_str(), session_id.c_str(), session_ver.c_str(), str_port.c_str(), str_port.c_str(), media_type.c_str());
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
            //int ret = eXosip_call_build_request(sip_context_, evtp->did, "INVITE", &msg);
            // 构建消息体
            char sdp[2048] = {0};
            {
                snprintf (sdp, 2048,
                "v=0\r\n"
                "o=%s 0 0 IN IP4 %s\r\n"
                "s=Play\r\n"
                "c=IN IP4 %s\r\n"
                "t=0 0\r\n"
                "m=%s %s %s 96\r\n"
                "a=sendonly\r\n"
                "a=rtpmap:96 PS/90000\r\n"
                "a=setup:passive\r\n"
                "a=connection:new\r\n"
                "y=%s\r\n", username.c_str(),client_ip.c_str(), client_ip.c_str(),
                media_type.c_str(), str_port.c_str(), str_proto.c_str(), ssrc.c_str());
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
        stream_id += Xzm::util::convert10to16(ssrc);
        char sz_url[256] = {0};
        snprintf(sz_url, 256,
         "http://%s/index/api/openRtpServer?secret=%s&port=%d&tcp_mode=%d&stream_id=%s",
        Server::instance()->GetServerInfo().rtp_ip.c_str(), secret.c_str(), 0, 0, stream_id.c_str());
        auto resp = requests::get(sz_url);
        auto ret_json = resp->GetJson();
        HV_JSON_GET_INT(ret_json, ret_code, "code");
        HV_JSON_GET_INT(ret_json, ser_port, "port");
        if (ret_code != 0) {
            return false;
        }
        client_ptr->talk_thread = std::thread([ser_port, client_ptr]() {
            client_ptr->is_talking.store(true);
            char cmd[256] = {0};
            snprintf(cmd, 256, "ffmpeg -re -stream_loop -1 -i \"./1.mp4\" -vcodec h264 -acodec copy -f rtp_mpegts rtp://10.23.132.27:%d", ser_port);
            CLOGE(BLUE, "publish_cmd:%s", cmd);
            system(cmd);    // 这里会阻塞，需要异步执行，且对话结束后，需要结束推流
        });
        client_ptr->talk_thread.detach();
        std::this_thread::sleep_for(std::chrono::seconds(3));
        return true;
        // 根据默认的stream_id构建出url
        char audio_url[128] = {0};
        snprintf(audio_url, 128, "rtsp://%s/rtp/%s", Server::instance()->GetServerInfo().rtp_ip.c_str(), stream_id.c_str());

        int is_udp = 1, pt=96, use_ps=0, only_audio=1;
        char publish_url[1024] = {0};
        snprintf(publish_url, 1024,
         "http://%s/index/api/startSendRtp?secret=%s&vhost=__defaultVhost__&app=rtp&stream=%s&ssrc=%s&dst_url=%s&dst_port=%s&is_udp=%d&pt=%d&use_ps=%d&only_audio=%d",
         Server::instance()->GetServerInfo().rtp_ip.c_str(),"Lsb4XJqAdK0QLVErbKEvBBGrSDJ3lexS",
          stream_id.c_str(), stream_id.c_str(),client_ip.c_str(),str_port.c_str(), is_udp, pt, use_ps, only_audio
         );
        CLOGI(YELLOW, "url:%s", publish_url);
        resp = requests::get(publish_url);
        ret_json = resp->GetJson();
        int publish_code = 0;
        HV_JSON_GET_INT(ret_json, publish_code, "code");
        if (publish_code != 0) {
            LOGE("startSendRtp failed!,code:%d", publish_code);
            return false;
        }
        std::cout << "\n----------------------------body:" << resp->body << std::endl;
        return true;
    }
};