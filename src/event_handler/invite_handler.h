/**
 * @file invite_handler.h
 * @author sleeping csleeping@163.com
 * @brief 客户端发起invite会话请求
 * @version 0.1
 * @date 2023-09-27
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#pragma once
#include "handler.h"

namespace Xzm
{
class InviteHandler : public Handler
{
public:
    InviteHandler();
    ~InviteHandler();

    virtual bool Process(eXosip_event_t *evtp, eXosip_t* sip_context_, int code) override;
};
};