/**
 * @file handler.h
 * @author sleeping (csleeping@163.com)
 * @brief 事件处理基类
 * @version 0.1
 * @date 2023-08-16
 *
 * @copyright Copyright (c) 2023
 *
 */

#pragma once

#include <memory>
#include <string>
#include <thread>
#include <unordered_map>

#include "../xzm_defines.h"
#include "boost/any.hpp"
#include "eXosip2/eXosip.h"

namespace Xzm {

// sip message处理函数原型
using FUNC_MSG_RESPONSE =
    std::function<void(eXosip_event_t *evtp, eXosip_t *sip_context_, int code, std::shared_ptr<boost::any> param)>;
extern std::unordered_map<std::string, FUNC_MSG_RESPONSE> msg_response_;

class Handler {
public:
    Handler();
    virtual ~Handler();
    virtual bool Process(eXosip_event_t *evtp, eXosip_t *sip_context_, int code);

    /* 断开摄像头推流到流服务器的连接 */
    int request_cancel_invite(eXosip_t *sip_context_, ClientRequestPtr req);
    /**
     * @brief 结束会话
     *
     * @param evtp
     * @param sip_context_
     * @return int
     */
    int request_bye(eXosip_event_t *evtp, eXosip_t *sip_context_);
    void response_message(eXosip_event_t *evtp, eXosip_t *sip_context_, int code);
    void response_message_answer(eXosip_event_t *evtp, eXosip_t *sip_context_, int code);
    int request_invite(eXosip_t *sip_context, ClientRequestPtr req);
    int request_invite_talk(eXosip_t *sip_context, ClientRequestPtr req);
    int request_device_query(eXosip_t *sip_context, ClientRequestPtr req);
    /* 刷新历史记录缓存 */
    int request_refresh_device_library(eXosip_t *sip_context, ClientRequestPtr req);
    /* 历史录像回放 */
    int request_invite_playback(eXosip_t *sip_context, ClientRequestPtr req);
    int request_broadcast(eXosip_t *sip_context, ClientRequestPtr req);
    /* 历史录像快进 */
    int request_fast_forward(eXosip_t *sip_context, ClientRequestPtr req);
    /* 历史录像快退 */
    int request_rewind(eXosip_t *sip_context, ClientRequestPtr req);
    /* 历史录像暂停 */
    int request_pasue(eXosip_t *sip_context, ClientRequestPtr req);
    int parse_xml(const char *data, const char *s_mark, bool with_s_make, const char *e_mark, bool with_e_make,
                  char *dest);

    int parse_device_xml(const std::string &xml_str);

    /**
     * @brief 解析报警报文
     *
     * @param str_xml [报警报文]
     * @return [0-正常解析]
     */
    int parse_alarm_xml(const std::string &str_xml);
    /**
     * @brief 解析历史录像记录
     *
     * @param xml_str
     * @return int
     */
    int parse_recordinfo_xml(const std::string &xml_str, bool &is_last_item);
    void dump_request(eXosip_event_t *evtp);
    void dump_response(eXosip_event_t *evtp);

    int get_random_sn();
    /**
     * @brief 生成对讲广播报文
     *
     * @return int
     */
    int generate_borad_cast_xml(char *str_from, char *str_to, char *str_body, ClientInfoPtr client_info_ptr,
                                ClientPtr client);
    void response_catalog(eXosip_event_t *evtp, eXosip_t *sip_context_, int code, std::shared_ptr<boost::any> param);
    void response_recordinfo(eXosip_event_t *evtp, eXosip_t *sip_context_, int code, std::shared_ptr<boost::any> param);
    void response_keepalive(eXosip_event_t *evtp, eXosip_t *sip_context_, int code, std::shared_ptr<boost::any> param);

    void response_alarm(eXosip_event_t *evtp, eXosip_t *sip_context_, int code, std::shared_ptr<boost::any> param);

private:
    std::atomic_bool is_print;
    static uint64_t sn_;                                       // 命令序列号
    std::map<DeviceID, std::atomic_int> history_video_cache_;  // <device_id, 当前解析出的历史录像个数>
    std::thread thread_;
};
using HandlerPtr = std::shared_ptr<Handler>;
};  // namespace Xzm
