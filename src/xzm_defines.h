/**
 * @file xzm_defines.h
 * @author sleeping (csleeping@163.com)
 * @brief 一些类型定义
 * @version 0.1
 * @date 2023-08-17
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#pragma once
#include <boost/thread/pthread/shared_mutex.hpp>
#include <string>
#include <iostream>
#include <memory>
#include <unordered_map>
#include <sstream>
#include <queue>
#include <boost/thread/locks.hpp>
#include <boost/thread/shared_mutex.hpp>
#include <codecvt>
#include <thread>

namespace Xzm
{

#define TALK_PREFIX "talk_"
/* 国标信令注册 */
#define BEGIN_REGISTER_EVENT_HANDLER    \
    event_map_ = std::unordered_map<eXosip_event_type, HandlerPtr>{
#define REGISTER_EVENT_HANDLER(event, handler)  \
    {event, std::make_shared<handler>()}
#define END_REGISTER_EVENT_HANDLER };

/* 国标消息处理函数定义 */
#define BEGIN_REGISTER_MSG_RESPONSE(resps)  \
    resps = {
#define REGISTER_MSG_RESPONSE(msg, handler, ptr) \
    {msg, std::bind(handler, ptr, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)},
#define END_REGISTER_MSG_RESPONSE()    \
    };
#define SIP_STRDUP(field) if (auth->field) (field) = osip_strdup_without_quote(auth->field)

/* libhv的json解析 */
#define HV_JSON_GET_INT(json, to, key) HV_JSON_GET_VALUE(json, to, key, number_integer)
#define HV_JSON_GET_STRING(json, to, key) HV_JSON_GET_VALUE(json, to, key, string)
#define HV_JSON_GET_BOOL(json, to, key) HV_JSON_GET_VALUE(json, to, key, boolean)
#define HV_JSON_GET_VALUE(json, to, key, type) auto iter_##to = json.find(key); \
    if (iter_##to->is_##type()) { \
        to = iter_##to.value(); \
    }
/* libhv逻辑处理函数注册 */
#define BEGIN_HV_REGISTER_HANDLER()   {
#define HV_REGISTER_SYNC_HANDLER(router, type, path, class, func, obj_ptr)    \
        router.type(path, std::bind(&class::func, obj_ptr, std::placeholders::_1, std::placeholders::_2));

#define HV_REGISTER_ASYNC_HANDLER(router, type, path, func, obj_ptr)    \
        router.type(path, [obj_ptr] (const HttpContextPtr& context) {   \
            return obj_ptr->func(context);  \
        });
#define END_HV_REGISTER_HANDLER() };

/* xml解析 */
#define XML_GET_STRING(node, name, value, temp_node, temp_text) \
    {   \
        temp_node = node->FirstChildElement(name);    \
        if (temp_node) {    \
            temp_text = temp_node->GetText();   \
            if (temp_text) {    \
                value = temp_text;   \
            }   \
        }   \
    }

#define XML_GET_INT(node, name, value, temp_node, temp_text) \
    {   \
        temp_node = node->FirstChildElement(name);    \
        if (temp_node) {    \
            temp_text = temp_node->GetText();   \
            if (temp_text) {    \
                value = std::stoi(temp_text);   \
            }   \
        }   \
    }

using DeviceID = std::string;

/* 读写锁定义 */
typedef boost::shared_mutex B_Lock;
typedef boost::unique_lock<B_Lock> WriteLock;
typedef boost::shared_lock<B_Lock> ReadLock;

enum ChannelType
{
    kChannelNone = 0,
    kChannelVideo,  // 视频通道
    kChannelAudio,  // 音频通道
    kChannelAlarm,  // 告警通道
    kChannelMax,
};

/* http状态码 */
enum HttpCode
{
    kHttpContinue = 100,                            // 继续。客户端应继续其请求
    kHttpSwitching_Protocols	= 101,              //  切换协议。服务器根据客户端的请求切换协议。只能切换到更高级的协议，例如，切换到HTTP的新版本协议
    kHttpOK = 200,                                  // 请求成功。一般用于GET与POST请求
    kHttpCreated = 201,                             // 已创建。成功请求并创建了新的资源
    kHttpAccepted = 202,                            // 已接受。已经接受请求，但未处理完成
    kHttpNonAuthoritativeInformation = 203,         // 非授权信息。请求成功。但返回的meta信息不在原始的服务器，而是一个副本
    kHttpNoContent = 204,                           // 无内容。服务器成功处理，但未返回内容。在未更新网页的情况下，可确保浏览器继续显示当前文档
    kHttpResetContent = 205,                        // 重置内容。服务器处理成功，用户终端（例如：浏览器）应重置文档视图。可通过此返回码清除浏览器的表单域
    kHttpPartialContent = 206,                      // 部分内容。服务器成功处理了部分GET请求
    kHttpMultipleChoices = 300,                     // 多种选择。请求的资源可包括多个位置，相应可返回一个资源特征与地址的列表用于用户终端（例如：浏览器）选择
    kHttpMovedPermanently = 301,                    // 永久移动。请求的资源已被永久的移动到新URI，返回信息会包括新的URI，浏览器会自动定向到新URI。今后任何新的请求都应使用新的URI代替
    kHttpFound = 302,                               // 临时移动。与301类似。但资源只是临时被移动。客户端应继续使用原有URI
    kHttpSeeOther = 303,                            // 查看其它地址。与301类似。使用GET和POST请求查看
    kHttpNotModified = 304,                         // 未修改。所请求的资源未修改，服务器返回此状态码时，不会返回任何资源。客户端通常会缓存访问过的资源，通过提供一个头信息指出客户端希望只返回在指定日期之后修改的资源
    kHttpUseProxy = 305,                            // 使用代理。所请求的资源必须通过代理访问
    kHttpUnused = 306,                              // 已经被废弃的HTTP状态码
    kHttpTemporaryRedirect = 307,                   // 临时重定向。与302类似。使用GET请求重定向
    kHttpBadRequest = 400,                          // 客户端请求的语法错误，服务器无法理解
    kHttpUnauthorized = 401,                        // 请求要求用户的身份认证
    kHttpPaymentRequired = 402,                     // 保留，将来使用
    kHttpForbidden = 403,                           // 服务器理解请求客户端的请求，但是拒绝执行此请求
    kHttpNotFound = 404,                            // 服务器无法根据客户端的请求找到资源（网页）。通过此代码，网站设计人员可设置"您所请求的资源无法找到"的个性页面
    kHttpMethodNotAllowed = 405,                    // 客户端请求中的方法被禁止
    kHttpNotAcceptable = 406,                       // 服务器无法根据客户端请求的内容特性完成请求
    kHttpProxyAuthenticationRequired = 407,         // 请求要求代理的身份认证，与401类似，但请求者应当使用代理进行授权
    kHttpRequestTimeOut = 408,                      // 服务器等待客户端发送的请求时间过长，超时
    kHttpConflict = 409,                            // 服务器完成客户端的 PUT 请求时可能返回此代码，服务器处理请求时发生了冲突
    kHttpGone = 410,                                // 客户端请求的资源已经不存在。410不同于404，如果资源以前有现在被永久删除了可使用410代码，网站设计人员可通过301代码指定资源的新位置
    kHttpLengthRequired = 411,                      // 服务器无法处理客户端发送的不带Content-Length的请求信息
    kHttpPreconditionFailed = 412,                  // 客户端请求信息的先决条件错误
    kHttpRequestEntityTooLarge = 413,               // 由于请求的实体过大，服务器无法处理，因此拒绝请求。为防止客户端的连续请求，服务器可能会关闭连接。如果只是服务器暂时无法处理，则会包含一个Retry-After的响应信息
    kHttpRequestURITooLarge = 414,                  // 请求的URI过长（URI通常为网址），服务器无法处理
    kHttpUnsupportedMediaType = 415,                // 服务器无法处理请求附带的媒体格式
    kHttpRequestedRangeNotSatisfiable = 416,        // 客户端请求的范围无效
    kHttpExpectationFailed = 417,                   // 服务器无法满足Expect的请求头信息
    kHttpInternalServerError = 500,                 // 服务器内部错误，无法完成请求
    kHttpNotImplemented = 501,                      // 服务器不支持请求的功能，无法完成请求
    kHttpBadGateway = 502,                          // 作为网关或者代理工作的服务器尝试执行请求时，从远程服务器接收到了一个无效的响应
    kHttpServiceUnavailable = 503,                  // 由于超载或系统维护，服务器暂时的无法处理客户端的请求。延时的长度可包含在服务器的Retry-After头信息中
    kHttpGatewayTimeOut = 504,                      // 充当网关或代理的服务器，未及时从远端服务器获取请求
    kHttpHTTPVersionNotSupported = 505,             // 服务器不支持请求的HTTP协议的版本，无法完成处理
};

// 摄像头推流状态
enum DevicePublishState
{
    kPublishNone = 0,   // 未知
    kPublishing,        // 推流中
    kPublishStop,       // 停止推流
};

// 客户端代理类型
enum XClientType
{
    kClientNone = 0,    // 未知
    kClientIPC,         // 网络摄像头
    kClientNVR,         // NVR设备
    kClientMax,         // 无
};

// 客户端请求类型
enum RequestType
{
    kRequestTypeNone = 0,
    kRequestTypeInvite, // 建立会话请求
    kRequestTypeCancel, // 断开会话请求
    kRequestTypeTalk,   // 建立对讲请求
    kRequestTypeCancelTalk, // 断开对讲请求
    kRequestTypeBroadcast,  // 对讲广播
    kRequestTypeScanDevice,     // 扫描设备信息
    kRequestTypeRefreshLibrary, // 刷新历史记录视频
    kRequestTypeQueryLibrary,   // 查询设备目录
    kRequestTypePlayback,       // 历史录像回放
    kRequestTypeFastforwardPlayback,    // 快进历史录像
    kRequestTypeMax = 9999,

};

/*国标xml查询类型*/
enum XmlQueryType
{
    kXmlQueryNone = 0,
    kXmlQueryFileLibrary,   // 文件目录检索
    kXmlQueryMax,
};

/* 国标设备类型*/
const static std::unordered_map<std::string, XClientType> kRegistedClientType = {
{"Embedded Net DVR/NVR/DVS", kClientNVR },
{"IP Camera", kClientIPC },
};

/* 实时流类型 */
enum LivingType
{
    kLivingTypeNone = 0,    // 未知
    kLivingTypeVideo,       // 视频流,包含音频流,用于前端观看
    kLivingTypeAudio,       // 音频流
    kLivingTypeTalkAudio,   // 对讲音频流
    kLivingTypeMax,
};

/* 实时流信息 */
struct LivingInfo
{
    std::string stream_id;
    std::string ip;
    unsigned short port;
    unsigned short talk_port;   // 流媒体服务用于推流的端口
    LivingType living_type;
};
using LivingInfoPtr = std::shared_ptr<LivingInfo>;
using LivingInfoMap = std::map<std::string, LivingInfoPtr>;

/* 推流信息 */
struct StreamInfo
{
    std::string ip;
    short port;
};

/* xml消息的查询参数信息 */
class XmlQueryParam
{
public:
    XmlQueryParam();
    //XmlQueryParam(XmlQueryType _q_type, const std::string& _cmd, uint64_t _sn, const std::string& _device_id);
    virtual ~XmlQueryParam();

    std::string cmd;
    uint64_t sn;
    std::string device_id;
    XmlQueryType query_type;
};
using XmlQueryParamPtr = std::shared_ptr<XmlQueryParam>;

/* 文件检索参数 */
class XmlQueryLibraryParam : public XmlQueryParam
{
public:
    XmlQueryLibraryParam();
    ~XmlQueryLibraryParam();

    std::string start_time;    // 必选 录像检索开始时间
    std::string end_time;      // 必选 录像检索终止时间
    std::string file_path;  // 可选 文件路径名
    std::string address;    // 可选 录像地址,支持不完全查询
    int secrecy = 0;    // 保密属性 0-不涉密 1-涉密
    std::string type;   // 录像产生类型 time/alarm/manual/all
    std::string recorder_id;    // 可选 录像触发者ID
    std::string indistinct_query;   // 可选 录像模糊查询属性,缺省为0 0-不进行模糊查询
    unsigned short stream_number = 0;      // 可选 码流编号 0-主码流 1-子码流 2-子码流 3-以此类推
    unsigned short alarm_method = 0;       // 可选 报警方式条件 0-全部
    unsigned short alarm_type = 0;         // 可选 报警类型
};
using XmlQueryLibraryParamPtr = std::shared_ptr<XmlQueryLibraryParam>;

/* xml消息体 */
class XmlQueryInfo
{
public:
    XmlQueryInfo()
    {

    }
    virtual ~XmlQueryInfo()
    {

    }

    std::string BuildMsg(const XmlQueryParamPtr& msg_in);
    virtual std::string ExtroXmlQueryParamfo(const XmlQueryParamPtr& msg_in);
};
using XmlQueryInfoPtr = std::shared_ptr<XmlQueryInfo>;

/* 文件检索消息体 */
class XmlQueryLibraryInfo : public XmlQueryInfo
{
public:
    XmlQueryLibraryInfo();
    ~XmlQueryLibraryInfo();

    virtual std::string ExtroXmlQueryParamfo(const XmlQueryParamPtr& msg_in) override;

};
using XmlQueryLibraryInfoPtr = std::shared_ptr<XmlQueryLibraryInfo>;

struct RecordInfo
{
    std::string parent_device_id;
    std::string name;
    std::string device_id;
    std::string file_path;
    std::string address;
    std::string start_time;
    std::string end_time;
    int secrecy = 0;
    std::string type;
    bool is_last_item = false;
    int current_num = 0;    // 当前是第几个记录
};
using RecordInfoPtr = std::shared_ptr<RecordInfo>;

/* 流媒体服务信息 */
struct MediaServerInfo
{
    std::string rtp_ip;
    unsigned short rtp_port;
    unsigned short rtp_proxy_port_min;
    unsigned short rtp_proxy_port_max;
};

/* sip服务器配置信息 */
struct ServerInfo
{
    std::string ua;     //服务器名称
    std::string nonce;  //随机数值
    std::string ip;
    unsigned short port;
    std::string sip_id;         //sip服务器ID
    std::string realm;          // sip域
    std::string passwd;         // 服务器密码
    unsigned int timeout;       // 超时间隔
    unsigned int valid_time;    // 有效时长

    std::string str()
    {
        std::stringstream ss;
        ss << std::endl << "\tua:\t" << ua
            << std::endl << "\tnonce:\t" << nonce
            << std::endl << "\tip:\t" << ip
            << std::endl << "\tport:\t" << port
            << std::endl << "\tsip_id:\t" << sip_id
            << std::endl << "\trealm:\t" << realm
            << std::endl << "\tpasswd:\t" << passwd
            << std::endl << "\ttimeout:\t" << timeout
            << std::endl << "\tvalid_time:\t" << valid_time;
        return ss.str();
    }
};
using ServerInfoPtr = std::shared_ptr<ServerInfo>;

struct HttpServerInfo
{
    std::string ip;
    unsigned short port;
    unsigned int work_threads;
    unsigned int work_process;
    unsigned int snap_cache_time;
};

struct ClientInfo
{
    std::string device_id;
    std::string name;
    std::string manufacturer;   // 厂商
    std::string model;          //IP Camera
    std::string owner;
    std::string civil_code;     // 3402000000
    std::string address;
    int parental = 0;
    std::string parent_id;      // parent_id和client中的device_id相同时，该设备具有语音输出功能
    int register_way = 1;
    int safety_way = 0;
    int secrecy = 0;
    int status = 1;
    ChannelType channel_type;
};
using ClientInfoPtr = std::shared_ptr<ClientInfo>;

// 客户端信息
struct Client
{
    Client(const std::string& _ip, unsigned short _port,
    const std::string& _device): ip(_ip),port(_port),device(_device),is_talking(false)
    {

    }

    Client(const std::string& _ip, unsigned short _port,
    const std::string& _device, bool _is_reg, unsigned short _rtp_port, XClientType _client_type):
    ip(_ip),port(_port),device(_device),is_reg(_is_reg),rtp_port(_rtp_port),client_type(_client_type),is_talking(false)
    {

    }

    ~Client() = default;

    std::string ip;
    unsigned short port = 0;
    std::string device; // 设备本身ID
    std::string real_device_id; // 实际操作id，如nvr播放则是通道id
    bool is_reg = false;
    unsigned short rtp_port = 10000;
    std::string ssrc;   // 10进制
    std::string rtsp_url;
    XClientType client_type = kClientNone;
    std::unordered_map<std::string, ClientInfoPtr> client_infos_;   // 每个设备有多个信息，如摄像头有视频和音频2个设备信息
    XmlQueryParamPtr param_ptr;
    std::thread talk_thread;    // 对话功能收音发送线程
    std::atomic_bool is_talking;    // 客户端是否正在对话
};
using ClientPtr = std::shared_ptr<Client>;

struct RequestParam
{
    RequestParam() {}
    virtual ~RequestParam() {}
};
using RequestParamPtr = std::shared_ptr<RequestParam>;

/* 查询历史录像参数 */
struct RequestParamQueryHistory : public RequestParam
{
    RequestParamQueryHistory() {}
    ~RequestParamQueryHistory() {}

    int start_time = 0;
    int end_time = 0;
};
using RequestParamQueryHistoryPtr = std::shared_ptr<RequestParamQueryHistory>;

/* 快进历史录像参数 */
struct RequestParamFastforward : public RequestParam
{
    RequestParamFastforward() {}
    ~RequestParamFastforward() {}

    std::string scale;
    std::string ssrc;
};
using RequestParamFastforwardPtr = std::shared_ptr<RequestParamFastforward>;

struct ClientRequest
{
    ClientPtr client_ptr;
    RequestType req_type;
    RequestParamPtr param_ptr;
    std::string ssrc;
};
using ClientRequestPtr = std::shared_ptr<ClientRequest>;
using RequestQueue = std::queue<ClientRequestPtr>;

};