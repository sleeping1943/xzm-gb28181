#include "call_message_answer_handler.h"
#include "../utils/log.h"

namespace Xzm
{

CallMessageAnswerHandler::CallMessageAnswerHandler()
{

}

CallMessageAnswerHandler::~CallMessageAnswerHandler()
{

}

bool CallMessageAnswerHandler::Process(eXosip_event_t *evtp, eXosip_t* sip_context_, int code)
{
    // 发送info信息控制回放时，不能回复200ok，回复200ok意为结束回放
    //osip_message_t* msg = nullptr;
    //int ret = eXosip_call_build_ack(sip_context_, evtp->did, &msg);
    dump_request(evtp);
    dump_response(evtp);
    //if (!ret && msg) {
    //    eXosip_call_send_ack(sip_context_, evtp->did, msg);
    //    LOGI("CallMessageAnswerHandler:eXosip_call_send_ack OK");
    //} else {
    //    LOGE("CallMessageAnswerHandler:eXosip_call_send_ack error=%d", ret);
    //}
    return true;
}
};