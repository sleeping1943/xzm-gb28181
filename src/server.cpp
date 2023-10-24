#include "server.h"
#include "event_handler/handler.h"
#include "include/rapidjson/document.h"
#include "utils/json_helper.h"
#include "utils/helper.h"
#include "utils/log.h"

#include <arpa/inet.h>
#include <functional>
#include <memory>
#include <mutex>
#include <utility>
#include "event_handler/register_handler.h"
#include "event_handler/call_answer_handler.h"
#include "event_handler/call_message_answer_handler.h"
#include "event_handler/invite_handler.h"
#include "xzm_defines.h"

/*

GET http://10.23.132.54:18080/query_device

*/


namespace Xzm
{

HandlerPtr Server::kDefaultHandler = std::make_shared<Handler>();
std::atomic_bool Server::is_server_quit(false);
std::atomic_bool Server::is_client_all_quit(false);

Server::Server():is_quit_(false), history_semaphore_(0)
{

}

Server::~Server()
{
    if (thread_.joinable()) {
        thread_.join();
    }
}

bool Server::Init(const std::string& conf_path)
{
    std::string content;
    if (!Xzm::util::read_file(conf_path, content)) {
        content = "read config error!";
        std::cerr << content << std::endl;
        return false;
    }
    std::cout << content << std::endl;
    if (!Xzm::Server::instance()->SetServerInfo(content)) {
        std::cerr << "json parse error!" << std::endl;
        return false;
    }
    // 分配流服务器对话端口
    unsigned short i = media_server_info_.rtp_proxy_port_min;
    for (; i <= media_server_info_.rtp_proxy_port_max; i++) {
        talk_ports_.push(i);
    }
    // 注册msg回调响应函数
    BEGIN_REGISTER_MSG_RESPONSE(msg_response_)
        REGISTER_MSG_RESPONSE("Catalog", &Handler::response_catalog, kDefaultHandler)
        REGISTER_MSG_RESPONSE("RecordInfo", &Handler::response_recordinfo, kDefaultHandler)
        REGISTER_MSG_RESPONSE("Keepalive", &Handler::response_keepalive, kDefaultHandler)
    END_REGISTER_MSG_RESPONSE()
    return true;
}

bool Server::SetServerInfo(const std::string& json_str)
{
    rapidjson::Document doc;
    JSON_PARSE_BOOL(doc, json_str.c_str());

    if (!doc.HasMember("sip_config") || !doc["sip_config"].IsObject()) {
        return false;
    }
    auto& sip_config = doc["sip_config"];
    JSON_VALUE_REQUIRE_STRING(sip_config, "ua", s_info_.ua);
    JSON_VALUE_REQUIRE_STRING(sip_config, "nonce", s_info_.nonce);
    JSON_VALUE_REQUIRE_STRING(sip_config, "ip", s_info_.ip);
    JSON_VALUE_REQUIRE_INT(sip_config, "port", s_info_.port);
    JSON_VALUE_REQUIRE_STRING(sip_config, "sipId", s_info_.sip_id);
    JSON_VALUE_REQUIRE_STRING(sip_config, "sipRealm", s_info_.realm);
    JSON_VALUE_REQUIRE_STRING(sip_config, "sipPass", s_info_.passwd);
    JSON_VALUE_REQUIRE_INT(sip_config, "sipTimeout", s_info_.timeout);
    JSON_VALUE_REQUIRE_INT(sip_config, "sipExpiry", s_info_.valid_time);

    auto& media_server_config = doc["media_server_config"];
    JSON_VALUE_REQUIRE_STRING(media_server_config, "rtp_ip", media_server_info_.rtp_ip);
    JSON_VALUE_REQUIRE_INT(media_server_config, "rtpPort", media_server_info_.rtp_port);
    JSON_VALUE_REQUIRE_INT(media_server_config, "rtp_proxy_port_min", media_server_info_.rtp_proxy_port_min);
    JSON_VALUE_REQUIRE_INT(media_server_config, "rtp_proxy_port_max", media_server_info_.rtp_proxy_port_max);
    return true;
}

bool Server::init_sip_server()
{
    register_event_handler();
    /* 
    step 1 
    申请结构体内存
    */
    sip_context_ = eXosip_malloc();
    if (!sip_context_) {
        LOG("eXosip_malloc error!");
        return false;
    }
    /*
     step 2
     初始化
    */
    if (eXosip_init(sip_context_)) {
        LOG("eXosip_init error");
        return false;
    }
    /*
      step 3
      开始监听
     */
    if (eXosip_listen_addr(sip_context_, IPPROTO_UDP, nullptr, s_info_.port, AF_INET, 0)) {
        LOG("eXosip_listen_addr error");
        return false;
    }

     eXosip_set_user_agent(sip_context_, s_info_.ua.c_str());
     /**
      step 4
      添加授权信息
      */
    if (eXosip_add_authentication_info(sip_context_, s_info_.sip_id.c_str(),
       s_info_.sip_id.c_str(), s_info_.passwd.c_str(), nullptr, s_info_.realm.c_str())) {
        LOG("eXosip_add_authentication_info error");
        return false;
    }
    return true;
}

bool Server::Start()
{
    if (!init_sip_server()) {
        return false;
    }
    thread_ = std::thread(std::bind(&Server::run, this));
    return true;
}

bool Server::Stop()
{
    is_quit_.store(true);
    return true;
}

bool Server::IsClientExist(const std::string& device)
{
    ReadLock _lock(client_mutex_);
    if (this->clients_.count(device) > 0) {
        return true;
    }
    return false;
}

bool Server::IsClientInfoExist(const std::string& device)
{
    ReadLock _lock(client_mutex_);
    for (const auto& iter : clients_) {
        const ClientPtr client = iter.second;
        for (const auto& iter_info : client->client_infos_) {
            const auto& client_info = iter_info.second;
            if (client_info->device_id == device) {
                return true;
            }
        }
    }
    return false;
}

ClientPtr Server::FindClient(const std::string& device)
{
    ClientPtr client_ptr = nullptr;
    ReadLock _lock(client_mutex_);
    if (clients_.count(device) > 0) {
        client_ptr = clients_[device];
    }
    return client_ptr;
}

ClientPtr Server::FindClientEx(const std::string& device)
{
    ClientPtr client_ptr = nullptr;
    ReadLock _lock(client_mutex_);
    if (clients_.count(device) > 0) {
        client_ptr = clients_[device];
    }
    for (const auto& iter : clients_) {
        const auto& client = iter.second;
        for (const auto& sub_iter : client->client_infos_) {
            if (sub_iter.second->device_id == device) {
                client_ptr = client;
                goto ret;
            }
        }
    }
ret:
    return client_ptr;
}
bool Server::AddClient(ClientPtr client)
{
    WriteLock _lock(client_mutex_);
    if (clients_.count(client->device) > 0) {
        return false;
    }
    clients_[client->device] = client;
    return true;
}

bool Server::UpdateClientInfo(const std::string& device_id,
 std::unordered_map<std::string, ClientInfoPtr> client_infos)
{
    WriteLock _lock(client_mutex_);
    auto device = clients_.begin();
    for (; device != clients_.end(); ++device) {
        if (device->second->device == device_id) {
            break;
        }
    }
    if (device == clients_.end()) { // 未找到对应的已注册设备
        return false;
    }
    for (const auto& iter : client_infos) {
        const auto& client_info = iter.second;
        auto& registed_clients = device->second->client_infos_;
        if (registed_clients.empty() || registed_clients.count(client_info->device_id) <= 0) {  // 没有该设备
            registed_clients[client_info->device_id] = client_info;
        }
    }
    //for (const auto& client_info : device->second->client_infos_) {
    //    if (device->second->client_infos_.count(client_info.second->device_id) <= 0) {   
    //        device->second->client_infos_[client_info.second->device_id] = client_info.second;
    //    }
    //}
    //device->second->client_infos_ = client_infos;
    return true;
}
bool Server::RemoveClient(const std::string& device)
{
    WriteLock _lock(client_mutex_);
    if (clients_.count(device) <= 0) {
        return false;
    }
    clients_.erase(device);
    return true;
}

void Server::ClearClient()
{
    ReadLock _lock(client_mutex_);
    clients_.clear();
    LOGI("already quit all clients...");
    return;
}

std::unordered_map<std::string, ClientPtr> Server::GetClients()
{
    ReadLock _lock(client_mutex_);
    decltype(clients_) ret_value = clients_;
    return ret_value;
}

int Server::AddRequest(const ClientRequestPtr req_ptr)
{
    std::lock_guard<std::mutex> _lock(req_mutex_);
    if (req_queue_.size() >= max_request_num) {
        return -1;
    }
    req_queue_.push(req_ptr);
    return 0;
}

void Server::AddRecordInfo(const std::string& parent_device_id, std::vector<RecordInfoPtr> records)
{
    WriteLock _lock(record_mutex_);
    for (const auto& record_info : records) {
        record_infos_[parent_device_id].emplace_back(record_info);
    }
}

std::vector<RecordInfoPtr> Server::GetRecordInfo(const std::string& parent_device_id)
{
    ReadLock _lock(record_mutex_);
    if (record_infos_.count(parent_device_id) > 0) {
        return record_infos_[parent_device_id];
    }
    return {};
}

void Server::RemoveRecordInfo(const std::string& parent_device_id)
{
    WriteLock _lock(record_mutex_);
    if (record_infos_.count(parent_device_id) > 0) {
        record_infos_.erase(parent_device_id);
    }
    return;
}

FUNC_MSG_RESPONSE Server::GetMsgResponse(const std::string& msg)
{
    FUNC_MSG_RESPONSE func = nullptr;
    if (msg_response_.count(msg) > 0) {
        func = msg_response_[msg];
    }
    return func;
}

void Server::WaitHistory()
{
    history_semaphore_.wait();
}

void Server::NotifyHistoryComplete()
{
    history_semaphore_.post();
}

void Server::AddPlacybackInfo(const std::string& ssrc, int did)
{
    WriteLock _lock(playback_mutex_);
    playback_infos_[ssrc] = did;
}

void Server::RemovePlaybackInfo(const std::string& ssrc)
{
    WriteLock _lock(playback_mutex_);
    playback_infos_.erase(ssrc);
}

int Server::GetPlaybackId(const std::string& ssrc)
{
    ReadLock _lock(playback_mutex_);
    if (playback_infos_.count(ssrc) > 0) {
        return playback_infos_[ssrc];
    }
    return -1;
}

LivingInfoPtr Server::FindLivingInfoPtr(const std::string& stream_id)
{
    ReadLock _lock(living_info_mutex_);
    if (living_info_map_.count(stream_id) > 0) {
        return living_info_map_[stream_id];
    }
    return nullptr;
}

void Server::AddLivingInfoPtr(const std::string& stream_id, LivingInfoPtr info)
{
    WriteLock _lock(living_info_mutex_);
    if (living_info_map_.count(stream_id) <= 0) {
        living_info_map_[stream_id] = info;
    }
    return;
}

void Server::DelLivingInfoPtr(const std::string& stream_id)
{
    WriteLock _lock(living_info_mutex_);
    if (living_info_map_.count(stream_id) > 0) {
        auto living_ptr = living_info_map_[stream_id];
        switch (living_ptr->living_type) {
            case kLivingTypeVideo:
            break;
            case kLivingTypeAudio:
            break;
            case kLivingTypeTalkAudio:
                if (living_ptr->talk_port >= media_server_info_.rtp_proxy_port_min
                && living_ptr->talk_port <= media_server_info_.rtp_proxy_port_max) {
                    ReleaseTalkPort(living_ptr->talk_port);
                } else {
                    LOGE("can not release talk_port[%d]", living_ptr->talk_port);
                }
            break;
            case kLivingTypeMax:
            case kLivingTypeNone:
            default:
            break;
        }
        living_info_map_.erase(stream_id);
    }
    return;
}

void Server::CleanLivingInfos()
{
    WriteLock _lock(living_info_mutex_);
    living_info_map_.clear();
}


std::pair<int, int> Server::FindPublishStreamInfo(const std::string& ssrc)
{
    ReadLock _lock(publish_streams_mutext_);
    if (publish_streams_.count(ssrc) > 0) {
        return publish_streams_[ssrc];
    }
    return std::make_pair(-1, -1);
}

void Server::AddPublishStreamInfo(const std::string& ssrc, int cid, int did)
{
    WriteLock _lock(publish_streams_mutext_);
    publish_streams_[ssrc] = std::make_pair(cid, did);
}

void Server::DelPublishStreamInfo(const std::string& ssrc)
{
    WriteLock _lock(publish_streams_mutext_);
    if (publish_streams_.count(ssrc) > 0) {
        publish_streams_.erase(ssrc);
    }
}

unsigned short Server::GetTalkPort()
{
    unsigned short port = 0;
    std::lock_guard<std::mutex> _lock(talk_mutex_);
    if (!talk_ports_.empty()) {
        port = talk_ports_.front();
        talk_ports_.pop();
    }
    return port;
}

void Server::ReleaseTalkPort(short port)
{
    if (port < media_server_info_.rtp_proxy_port_min
    || port > media_server_info_.rtp_proxy_port_max) {
        return;
    }
    std::lock_guard<std::mutex> _lock(talk_mutex_);
    talk_ports_.push(port);
}

bool Server::run()
{
    while (true) {
        {
            ReadLock _lock(client_mutex_);
            if (is_quit_ && clients_.empty()) {
                is_client_all_quit = true;
                LOGI("aleady quit all clients......");
                break;
            }
        }
        eXosip_event_t *evtp = eXosip_event_wait(sip_context_, 0, 20);  // 接受时间20ms超时
        process_request();
        if (!evtp) {
            eXosip_automatic_action(sip_context_);  // 执行一些自动操作
            osip_usleep(100000);
            continue;
        }
        eXosip_automatic_action(sip_context_);  // 执行一些自动操作
        /*
         handler the event here
         应该有个默认的处理函数体,即使没有注册处理函数，也不会没有响应
         */
        std::shared_ptr<Handler> handler_ptr = kDefaultHandler;
        if (event_map_.count(evtp->type) > 0) {
            handler_ptr = event_map_[evtp->type];
        }
        if (handler_ptr) {
            handler_ptr->Process(evtp, sip_context_,200);
        }
        eXosip_event_free(evtp);    // 释放事件所占资源
    }
    
    //ClearClient();
    return true;
}

bool Server::register_event_handler()
{
    BEGIN_REGISTER_EVENT_HANDLER 
        REGISTER_EVENT_HANDLER(EXOSIP_MESSAGE_NEW, RegisterHandler), // 新客户端发送请求
        REGISTER_EVENT_HANDLER(EXOSIP_CALL_ANSWERED, CallAnswerHandler), // 宣布通话开始
        REGISTER_EVENT_HANDLER(EXOSIP_CALL_MESSAGE_ANSWERED, CallMessageAnswerHandler), // 宣布通话正常有效
        REGISTER_EVENT_HANDLER(EXOSIP_CALL_INVITE, InviteHandler), // 宣布通话正常有效

    END_REGISTER_EVENT_HANDLER 
        //{ EXOSIP_MESSAGE_NEW, std::make_shared<RegisterHandler>()}, // 新客户端发送请求
        //{ EXOSIP_CALL_MESSAGE_NEW, nullptr},
        //{ EXOSIP_CALL_CLOSED, nullptr},
        //{ EXOSIP_CALL_RELEASED, nullptr},
        //{ EXOSIP_MESSAGE_NEW, std::make_shared<RegisterHandler>()}, // 新客户端发送请求
        //{ EXOSIP_MESSAGE_ANSWERED, nullptr},
        //{ EXOSIP_MESSAGE_REQUESTFAILURE, nullptr},
        //{ EXOSIP_CALL_INVITE, nullptr},
        //{ EXOSIP_CALL_PROCEEDING, nullptr},
        //{ EXOSIP_CALL_ANSWERED, nullptr},
        //{ EXOSIP_CALL_SERVERFAILURE, nullptr},
        //{ EXOSIP_IN_SUBSCRIPTION_NEW, nullptr},
    return true;
}

int Server::process_request()
{
    if (!sip_context_ || !kDefaultHandler) {
        return -1;
    }
    std::lock_guard<std::mutex> _lock(req_mutex_);
    while (!req_queue_.empty()) {
        auto client_req = req_queue_.front();
        switch (client_req->req_type) {
        case kRequestTypeNone:
        break;
        case kRequestTypeInvite: // 建立会话请求
            CLOGI(RED, "process request send invite................................");
            kDefaultHandler->request_invite(sip_context_, client_req);
        break;
        case kRequestTypeMax:
        break;
        case kRequestTypeCancel:
            CLOGI(RED, "process request cancel invite................................");
            kDefaultHandler->request_cancel_invite(sip_context_, client_req);
        break;
        case kRequestTypeTalk:  // 开启对话请求
            CLOGI(RED, "process request send invite................................");
            kDefaultHandler->request_invite_talk(sip_context_, client_req);
        break;
        case kRequestTypeCancelTalk:
        break;
        case kRequestTypeBroadcast:
            CLOGI(RED, "process request broadcast..................................");
            kDefaultHandler->request_broadcast(sip_context_, client_req);
        break;
        case kRequestTypeScanDevice:
            CLOGI(RED, "process request scan device..................................");
            kDefaultHandler->request_device_query(sip_context_, client_req);
        break;
        case kRequestTypeQueryLibrary:
            CLOGI(RED, "process request query library..................................");
        break;
        case kRequestTypeRefreshLibrary:
            CLOGI(RED, "process request refresh library..................................");
            kDefaultHandler->request_refresh_device_library(sip_context_, client_req);
        break;
        case kRequestTypePlayback:
            CLOGI(RED, "process request playback..................................");
            kDefaultHandler->request_invite_playback(sip_context_, client_req);
        break;
        case kRequestTypeFastforwardPlayback:
            CLOGI(RED, "process request fast forward playback..................................");
            kDefaultHandler->request_fast_forward(sip_context_, client_req);
        break;
        default:
        break;
        }
        req_queue_.pop();
    }

    return 0;
}


};