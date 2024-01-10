/**
 * @file server.h
 * @author sleeping (csleeping@163.com)
 * @brief 国标28181-2016 服务端
 * @version 0.1
 * @date 2023-08-14
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#pragma once
#include <iostream>
#include <sstream>
#include "utils/singleton.h"
#include <atomic>
#include <thread>
#include <unordered_map>
#include "event_handler/handler.h"
#include <boost/thread/shared_mutex.hpp>
#include <boost/interprocess/sync/interprocess_semaphore.hpp>

extern "C" {
#include <eXosip2/eXosip.h>
#include <osip2/osip_mt.h>
}
#include "xzm_defines.h"

#define LOG(x) std::cout << x << std::endl;

namespace Xzm
{

class Server : public util::Singleton<Server>
{
    friend class Singleton;
#ifdef WIN32
public:
#endif
#ifdef LINUX
private:
#endif
    Server();

public:
    ~Server();
    inline void Test()
    {
        std::cout << "server Test" << std::endl;
    }

    bool Init(const std::string& conf_path);
    bool SetServerInfo(const std::string& json_str);
    inline ServerInfo GetServerInfo() { return s_info_; }
    inline MediaServerInfo GetMediaServerInfo() { return media_server_info_; }
    inline struct eXosip_t* GetSipContext() { return sip_context_; }

    bool Start();
    bool Stop();
    bool IsClientExist(const std::string& device);
    bool IsClientInfoExist(const std::string& device);
    ClientPtr FindClient(const std::string& device);
    /**
     * @brief device可能是通道id,同时在通道中匹配
     * 
     */
    ClientPtr FindClientEx(const std::string& device);
    bool AddClient(ClientPtr client);
    bool UpdateClientInfo(const std::string& device_id,
     std::unordered_map<std::string, ClientInfoPtr> client_infos);
    bool RemoveClient(const std::string& device);
    void ClearClient();
    std::unordered_map<std::string, ClientPtr> GetClients();
    int AddRequest(const ClientRequestPtr req_ptr);
    void AddRecordInfo(const std::string& parent_device_id, std::vector<RecordInfoPtr> records);
    std::vector<RecordInfoPtr> GetRecordInfo(const std::string& parent_device_id);
    void RemoveRecordInfo(const std::string& parent_device_id);

    FUNC_MSG_RESPONSE GetMsgResponse(const std::string& msg);

    /* 等待历史录像查询完成 */
    void WaitHistory();

    /* 历史录像查询完成通知 */
    void NotifyHistoryComplete();

    void AddPlacybackInfo(const std::string& ssrc, int did);
    void RemovePlaybackInfo(const std::string& ssrc);
    int GetPlaybackId(const std::string& ssrc);

    LivingInfoPtr FindLivingInfoPtr(const std::string& stream_id);
    void AddLivingInfoPtr(const std::string& stream_id, LivingInfoPtr info);
    void DelLivingInfoPtr(const std::string& stream_id);
    void CleanLivingInfos();

    void AddStream(const std::string& stream_id, StreamInfoPtr info_ptr);
    StreamInfoPtr GetStreamInfo(const std::string& stream_id);
    bool IsStreamValid(const std::string& stream_id);
    void DelStream(const std::string& stream_id);

    std::pair<int, int> FindPublishStreamInfo(const std::string& ssrc);

    /**
    * @brief 添加推流记录
    * 
    * @param ssrc stream_id
    * @param cid call id of call
    * @param did dialog id of call
    */
    void AddPublishStreamInfo(const std::string& ssrc, int cid, int did);
    void DelPublishStreamInfo(const std::string& ssrc);
    // 获取对讲的流服务器端口
    unsigned short GetTalkPort();
    // 回收释放对讲的流服务器端口
    void ReleaseTalkPort(short port);

public:
    static HandlerPtr kDefaultHandler;

    std::map<std::string, bool> living_states_; // <id, state>  对应id的摄像头是否服务器已经在对其推流,id即ssrc

private:
    bool init_sip_server();
    /**
     * @brief sip服务处理线程函数体
     * 
     * @return true 
     * @return false 
     */
    bool run();

    bool register_event_handler();
    /**
     * @brief 处理客户端的http请求
     * 
     * @return int 
     */
    int process_request();


public:
    static std::atomic_bool is_server_quit;
    static std::atomic_bool is_client_all_quit;
    //static std::unordered_map<std::string, StreamInfo> stream_infos_;   // <stream_id, StreamInfo>,stream_id以"talk_"开头

private:
    std::atomic_bool is_quit_;
    ServerInfo s_info_;
    MediaServerInfo media_server_info_;
    struct eXosip_t *sip_context_;
    std::thread thread_;
    std::unordered_map<eXosip_event_type, HandlerPtr> event_map_; // 注册的事件处理函数体
    std::unordered_map<std::string, ClientPtr> clients_;  // 已注册的客户端 <device_id, std::shared_ptr<Client>>
    std::unordered_map<std::string, std::vector<RecordInfoPtr>> record_infos_;  // 缓存的历史录像记录 <device_id, std::vector<RecordInfoPtr>>

    B_Lock client_mutex_;
    B_Lock record_mutex_;

    RequestQueue req_queue_;
    unsigned int max_request_num = 1000;
    std::mutex req_mutex_;
    boost::interprocess::interprocess_semaphore history_semaphore_;
    
    B_Lock playback_mutex_;
    std::map<std::string, int> playback_infos_; // <ssrc, dialog_id>

    B_Lock living_info_mutex_;
    LivingInfoMap living_info_map_;

    B_Lock publish_streams_mutext_;
    std::unordered_map<std::string, std::pair<int, int>> publish_streams_;   // 正在推流直播的rtsp, <ssrc, <cid, did>>
    std::mutex talk_mutex_;
    std::queue<unsigned short> talk_ports_;


    B_Lock valid_stream_mutex_;
    std::mutex stream_mtx_;
    std::unordered_map<std::string, StreamInfoPtr> stream_infos_;  // <stream_id, stream_info>
};
#define gServer Server::instance()
};