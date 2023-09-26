/**
 * @file call_message_answer_handler.h
 * @author sleeping csleeping@163.com
 * @brief 对话正常且有效，默认回复200ok
 * @version 0.1
 * @date 2023-09-26
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#pragma once
#include "handler.h"

namespace Xzm
{
class CallMessageAnswerHandler : public Handler
{
public:
    CallMessageAnswerHandler();
    ~CallMessageAnswerHandler();

    virtual bool Process(eXosip_event_t *evtp, eXosip_t* sip_context_, int code) override;
};
};