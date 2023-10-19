#include "call_answer_handler.h"
#include "../utils/log.h"
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/split.hpp>
#include <osipparser2/osip_parser.h>
#include <boost/algorithm/string.hpp>
#include <boost/regex.h>
#include <boost/algorithm/string/regex.hpp>
#include "../utils/helper.h"
#include "../server.h"

namespace Xzm
{

CallAnswerHandler::CallAnswerHandler()
{

}

CallAnswerHandler::~CallAnswerHandler()
{

}

bool CallAnswerHandler::Process(eXosip_event_t *evtp, eXosip_t* sip_context_, int code)
{
    osip_message_t* msg = nullptr;
    int ret = eXosip_call_build_ack(sip_context_, evtp->did, &msg);
    osip_call_id_t *call_id = osip_message_get_call_id(evtp->request);
    std::cout << "--------------------------------------->did:" << evtp->did << std::endl;
    osip_body_t *body = nullptr;
    osip_message_get_body(evtp->request, 0, &body);
    std::cout << "-------------------------------------body:" << body->body << std::endl;
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
                std::cout << "----------------------------------------" << std::endl;
                ssrc = str.substr(2);
                ssrc = Xzm::util::convert10to16(ssrc);
                std::cout << "ssrc:" << ssrc << std::endl;
                break;
            }
        }
        if (!ssrc.empty()) {
            Server::instance()->AddPlacybackInfo(ssrc, evtp->did);
        }
    }
    dump_request(evtp);
    dump_response(evtp);
    if (!ret && msg) {
        eXosip_call_send_ack(sip_context_, evtp->did, msg);
        gServer->AddPublishStreamInfo(ssrc, evtp->cid, evtp->did);
        LOGI("eXosip_call_send_ack OK, Add PublishStreamInfo, ssrc[%s], cid[%d], did[%d]", ssrc.c_str(), evtp->cid, evtp->did);
    } else {
        LOGE("eXosip_call_send_ack error=%d", ret);
    }
    return true;
}
};