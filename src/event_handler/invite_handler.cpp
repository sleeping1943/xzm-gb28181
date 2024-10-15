#include "invite_handler.h"
#include "../utils/log.h"
#include <boost/algorithm/string.hpp>
#include <boost/regex.h>
#include <boost/algorithm/string/regex.hpp>
#include <cctype>
#include <memory>
#include <osipparser2/headers/osip_header.h>
#include <osipparser2/headers/osip_via.h>
#include <osipparser2/sdp_message.h>
#include <ostream>
#include "../server.h"
#include <hv/requests.h>
#include <stdlib.h>
#include "../utils/helper.h"
#include "../utils/md5.h"
#include "../utils/config.h"
#include "fmt/format.h"

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

        SendRtpInfoPtr send_info_ptr = std::make_shared<SendRtpInfo>();
        send_info_ptr->ssrc = parse_ssrc(str_body);
        send_info_ptr->secret = gMediaServerInfo.secret;
        send_info_ptr->stream_id = TALK_PREFIX;
        send_info_ptr->app = "rtp";
        send_info_ptr->host = "__defaultVhost__";
        if (!replay_invite(evtp, sip_context_, body, send_info_ptr)) {
            return false;
        }
        /* 发送openRtpServer的http请求,确定发送端口
           注: client_ip和client_port已在replay_invite中解析出
           本地测试
        */
        //send_info_ptr->client_ip = "10.23.132.56";
        //send_info_ptr->client_port = 8020;
        if (!send_meida_data(send_info_ptr)) {
            return false;
        }

        return true;
    }

    bool InviteHandler::send_meida_data(const SendRtpInfoPtr ptr)
    {
        // 以下推流到摄像头逻辑修改为推流后执行
        std::string publish_url = fmt::format("http://{}/index/api/startSendRtp?secret={}&vhost={}&app={}" 
                                        "&stream={}&src_port={}&ssrc={}&dst_url={}&dst_port={}&is_udp={}&pt={}&use_ps={}&only_audio={}"
                                        , gMediaServerInfo.rtp_ip , ptr->secret , ptr->host , ptr->app, ptr->stream_id, ptr->talk_port
                                        , ptr->stream_id, ptr->client_ip , ptr->client_port , ptr->is_udp, ptr->pt, ptr->use_ps, ptr->only_audio);
        CLOGI(YELLOW, "url:%s", publish_url.c_str());
        auto resp = requests::get(publish_url.c_str());
        auto ret_json = resp->GetJson();
        int publish_code = 0;
        HV_JSON_GET_INT(ret_json, publish_code, "code");
        if (publish_code != 0) {
            LOGE("startSendRtp failed!,code:%d", publish_code);
            return false;
        }
        return true;
    }

    std::string InviteHandler::parse_ssrc(const std::string& str_body)
    {
        std::vector<std::string> str_vec;
        std::string ssrc, client_ssrc;
        if (str_body.empty()) {
            return ssrc;
        }
        boost::split_regex(str_vec, str_body, boost::regex("\r\n"));
        for (auto& str : str_vec) {
            std::transform(str.begin(), str.end(), str.begin(), [] (int c) {
                return std::tolower(c);
            });
            if (boost::starts_with(str, "y=")) {
                ssrc = str.substr(2);
            } 
        }
        if (!ssrc.empty()) {
            CLOGI(YELLOW, "ssrc :%s", ssrc.c_str());
        }
        return ssrc;
    }

    bool InviteHandler::replay_invite(eXosip_event_t *evtp, eXosip_t* sip_context_, osip_body_t *body, SendRtpInfoPtr send_info_ptr)
    {
        sdp_message_t *sdp_msg = nullptr;
        std::string username, session_id, session_ver, str_proto, media_type;
        ClientPtr client_ptr = nullptr;
        do {
            auto s_info = gServerInfo;
            sdp_message_init(&sdp_msg);
            sdp_message_parse(sdp_msg, body->body);
            send_info_ptr->client_ip = sdp_message_o_addr_get(sdp_msg);
            send_info_ptr->client_port = std::atoi(sdp_message_m_port_get(sdp_msg, 0));
            username = sdp_message_o_username_get(sdp_msg);
            session_id = sdp_message_o_sess_id_get(sdp_msg);
            session_ver = sdp_message_o_sess_version_get(sdp_msg);
            str_proto = sdp_message_m_proto_get(sdp_msg, 0);
            media_type = sdp_message_m_media_get(sdp_msg, 0);

            std::string str_client_info = fmt::format("\nclient_ip:{}\tusername:{}\tsession_id:{}\tsession_ver:{}\tclient_port:{}\tstr_proto:{}\tmedia_type:{}\n" 
                                                , send_info_ptr->client_ip, username, session_id, session_ver
                                                , send_info_ptr->client_port, str_proto, media_type);
            CLOGI(RED,"sip_client_info:%s", str_client_info.c_str());
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
            int ret = eXosip_call_build_answer(sip_context_, evtp->tid, 200, &msg);
            if (ret != 0) {
                LOGE("InviteHandler:eXosip_call_build_ack failed!");
                break;
            }
            send_info_ptr->talk_port = Server::instance()->GetTalkPort();
            if (send_info_ptr->talk_port <= 0) {
                LOGE("can not get port for mediakit to speek");
                break;
            }
            auto md5_str = Xzm::MD5(username).toStr();
            std::transform(md5_str.begin(), md5_str.end(), md5_str.begin(), [](const char& c) {
                return std::toupper(c);
            });
            send_info_ptr->stream_id += md5_str.substr(md5_str.length() - 8);
            auto media_info = gMediaServerInfo;
            // 构建消息体
            char sdp[2048] = {0};
            {
                snprintf (sdp, 2048,
                "v=0\r\n"
                "o=%s 0 0 IN IP4 %s\r\n"
                "s=Play\r\n"
                "c=IN IP4 %s\r\n"
                "t=0 0\r\n"
                "m=%s %d %s 8 \r\n"
                "a=sendonly\r\n"
                "a=rtpmap:8 PCMA/8000\r\n"
                "y=%s\r\n"
                "f=v/////a/1/8/1\r\n", s_info.sip_id.c_str(), media_info.rtp_ip.c_str(), media_info.rtp_ip.c_str(),
                media_type.c_str(), send_info_ptr->talk_port, str_proto.c_str(), send_info_ptr->ssrc.c_str());
            }
            osip_message_set_body(msg, sdp, strlen(sdp));
            osip_message_set_content_type(msg, "application/sdp");
            ret = eXosip_call_send_answer(sip_context_, evtp->tid, 200, msg);
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
        return true;
    }
};