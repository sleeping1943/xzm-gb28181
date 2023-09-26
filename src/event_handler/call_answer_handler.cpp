#include "call_answer_handler.h"
#include "../utils/log.h"
#include <osipparser2/osip_parser.h>

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
    //std::cout << "------------------------------------->>call_id:number["
    //<< call_id->number
    //<< ",host:" << call_id->host << "]" << std::endl;
    //osip_cseq_t *cseq = osip_message_get_cseq(evtp->request);
    //std::cout << "--------------------------------------->cseq:" << cseq->number << "," << cseq->method << std::endl;
    std::cout << "--------------------------------------->did:" << evtp->did << std::endl;
    dump_request(evtp);
    dump_response(evtp);
    if (!ret && msg) {
        eXosip_call_send_ack(sip_context_, evtp->did, msg);
        LOGI("eXosip_call_send_ack OK");
    } else {
        LOGE("eXosip_call_send_ack error=%d", ret);
    }
    return true;
}
};