#include "http_server.h"
#include "../include/rapidjson/document.h"
#include "../utils/json_helper.h"
#include "../utils/helper.h"
#include "../utils/log.h"
#include <chrono>
#include <functional>
#include <hv/HttpServer.h>
#include <mutex>
#include <ostream>
#include <thread>
#include "../server.h"
#include "../utils/timer.h"
#include "hv/hasync.h"
#include "hv/requests.h"

namespace Xzm
{
XHttpServer::XHttpServer():is_quit_(false)
{

}

XHttpServer::~XHttpServer()
{

}

bool XHttpServer::Init(const std::string& conf_path)
{
    std::string content;
    if (!Xzm::util::read_file(conf_path, content)) {
        return false;
    }
    rapidjson::Document doc;
    JSON_PARSE_BOOL(doc, content.c_str());
    if (!doc.HasMember("http_config") || !doc["http_config"].IsObject()) {
        return false;
    }
    auto& http_config = doc["http_config"];
    JSON_VALUE_REQUIRE_STRING(http_config, "ip", s_info_.ip);
    JSON_VALUE_REQUIRE_INT(http_config, "port", s_info_.port);
    JSON_VALUE_REQUIRE_INT(http_config, "work_threads", s_info_.work_threads);
    JSON_VALUE_REQUIRE_INT(http_config, "work_process", s_info_.work_process);
    JSON_VALUE_OPTION_INT(http_config, "snap_cache_time", s_info_.snap_cache_time, 3600);

    server_.service = &router;
    server_.port = s_info_.port;
    server_.worker_processes = s_info_.work_process;
    server_.worker_threads = s_info_.work_threads;


    BEGIN_HV_REGISTER_HANDLER()
        /* 同步请求 */
        HV_REGISTER_SYNC_HANDLER(router, GET, "/query_device", XHttpServer, query_device_list, this);
        HV_REGISTER_SYNC_HANDLER(router, GET, "/start_rtsp_publish", XHttpServer, start_rtsp_publish, this);
        HV_REGISTER_SYNC_HANDLER(router, GET, "/stop_rtsp_publish", XHttpServer, stop_rtsp_publish, this);
        HV_REGISTER_SYNC_HANDLER(router, GET, "/start_invite_talk", XHttpServer, start_invite_talk, this);
        HV_REGISTER_SYNC_HANDLER(router, GET, "/start_talk_broadcast", XHttpServer, start_talk_broadcast, this);
        HV_REGISTER_SYNC_HANDLER(router, GET, "/scan_device_list", XHttpServer, scan_device_list, this);
        HV_REGISTER_SYNC_HANDLER(router, GET, "/query_device_library", XHttpServer, query_device_library, this);
        HV_REGISTER_SYNC_HANDLER(router, GET, "/refresh_device_library", XHttpServer, refresh_device_library, this);
        HV_REGISTER_SYNC_HANDLER(router, GET, "/start_playback", XHttpServer, start_playback, this);
        HV_REGISTER_SYNC_HANDLER(router, GET, "/fast_forward_playback", XHttpServer, fast_forward_playback, this);
        HV_REGISTER_SYNC_HANDLER(router, POST, "/on_publish", XHttpServer, on_publish, this);
        HV_REGISTER_SYNC_HANDLER(router, POST, "/on_play", XHttpServer, on_play, this);
        HV_REGISTER_SYNC_HANDLER(router, GET, "/get_snap", XHttpServer, get_snap, this);

        /* 异步请求 */
        HV_REGISTER_ASYNC_HANDLER(router, GET, "/refresh_device_library_async", refresh_device_library_async, this);

        //router.POST("/refresh_device_library_async", std::bind(&XHttpServer::refresh_device_library_async, this, std::placeholders::_1));
        //router.POST("/11", std::bind(&XHttpServer::refresh_device_library_async, this, std::placeholders::_1));
    END_HV_REGISTER_HANDLER()

    std::stringstream ss;
    ss << "http.worker_processes    :" << server_.worker_processes << std::endl
       << "http.worker_connectons   :" << server_.worker_connections << std::endl
       << "http.worker_threads      :" << server_.worker_threads;
    CLOGI(YELLOW, "\n%s\n", ss.str().c_str());
    return true;
}

bool XHttpServer::Start()
{
    Run();
    return true;
}

bool XHttpServer::Stop()
{
    http_server_stop(&server_);
    is_quit_ = true;
    return true;
}

bool XHttpServer::Run()
{
    thread_ = std::thread([this]() {
        http_server_run(&server_);
    });
    return true;
}

int XHttpServer::scan_device_list(HttpRequest* req, HttpResponse* resp)
{
    std::string device_id = req->GetParam("device_id");
    if (device_id.empty()) {
        return resp->String(get_simple_info(400, "错误的device_id"));
    }
    auto client_ptr = Server::instance()->FindClient(device_id);
    if (!client_ptr) {
        return resp->String(get_simple_info(101, "can not find the device client"));
    }
    auto req_ptr = std::make_shared<ClientRequest>();
    req_ptr->client_ptr = client_ptr;
    req_ptr->req_type = kRequestTypeScanDevice;
    Server::instance()->AddRequest(req_ptr);
    resp->json["code"] = 0;
    resp->json["data"]["action"] = "scan_device_list";
    resp->json["msg"] = "success";
    return kHttpOK;
}

int XHttpServer::query_device_list(HttpRequest* req, HttpResponse* resp)
{
    std::string device_list;
    rapidjson::Document doc(rapidjson::kObjectType);    // doc.SetObject();
    rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();
    std::vector<ClientPtr> vec_device;
    const auto& clients = Server::instance()->GetClients();
    rapidjson::Value arr_device(rapidjson::kArrayType);
    for (const auto& iter : clients) {
        ClientPtr device = iter.second;
        rapidjson::Value value(rapidjson::kObjectType);
        rapidjson::Value d_name(rapidjson::kStringType);
        rapidjson::Value d_ip(rapidjson::kStringType);
        rapidjson::Value d_port(rapidjson::kNumberType);
        rapidjson::Value d_ssrc(rapidjson::kStringType);
        rapidjson::Value d_rtsp_url(rapidjson::kStringType);
        rapidjson::Value d_dev_type(rapidjson::kStringType);
        std::string str_type = "未知";
        switch (device->client_type) {
            case kClientNVR:
                str_type = "NVR设备";
            break;
            case kClientIPC:
                str_type = "网络摄像头";
            break;
            default:
                str_type = "未知";
            break;
        }
        d_name.SetString(device->device.c_str(), allocator);
        d_ip.SetString(device->ip.c_str(), allocator);
        d_port.SetInt(device->port);
        d_ssrc.SetString(device->ssrc.c_str(), allocator);
        d_rtsp_url.SetString(device->rtsp_url.c_str(), allocator);
        d_dev_type.SetString(str_type.c_str(), allocator);
        //value.AddMember("name", device->device.c_str(), allocator);
        value.AddMember("name", d_name, allocator);
        value.AddMember("ip", d_ip, allocator);
        value.AddMember("port", d_port, allocator);
        value.AddMember("ssrc", d_ssrc, allocator);
        value.AddMember("rtsp", d_rtsp_url, allocator);
        value.AddMember("type", d_dev_type, allocator);
        rapidjson::Value arr_client_info(rapidjson::kArrayType);
        for (const auto& obj : device->client_infos_) {
            auto client_info = obj.second;
            if (client_info->channel_type != kChannelVideo) {   // 暂只返回视频通道
                continue;
            }
            rapidjson::Value value(rapidjson::kObjectType);
            rapidjson::Value device_id(rapidjson::kStringType);
            rapidjson::Value name(rapidjson::kStringType);
            rapidjson::Value manufacturer(rapidjson::kStringType);
            rapidjson::Value model(rapidjson::kStringType);
            rapidjson::Value owner(rapidjson::kStringType);
            rapidjson::Value civil_code(rapidjson::kStringType);
            rapidjson::Value address(rapidjson::kStringType);
            rapidjson::Value parent_id(rapidjson::kStringType);
            rapidjson::Value parental(rapidjson::kNumberType);
            rapidjson::Value register_way(rapidjson::kNumberType);
            rapidjson::Value safety_way(rapidjson::kNumberType);
            rapidjson::Value secrecy(rapidjson::kNumberType);
            rapidjson::Value status(rapidjson::kNumberType);

            device_id.SetString(client_info->device_id.c_str(), allocator);
            name.SetString(client_info->name.c_str(), allocator);
            manufacturer.SetString(client_info->manufacturer.c_str(), allocator);
            model.SetString(client_info->model.c_str(), allocator);
            owner.SetString(client_info->owner.c_str(), allocator);
            civil_code.SetString(client_info->civil_code.c_str(), allocator);
            address.SetString(client_info->address.c_str(), allocator);
            parent_id.SetString(client_info->parent_id.c_str(), allocator);
            parental.SetInt(client_info->parental);
            register_way.SetInt(client_info->register_way);
            safety_way.SetInt(client_info->safety_way);
            secrecy.SetInt(client_info->secrecy);
            status.SetInt(client_info->status);

            value.AddMember("device_id", device_id, allocator);
            value.AddMember("name", name, allocator);
            value.AddMember("manufacturer", manufacturer, allocator);
            value.AddMember("model", model, allocator);
            value.AddMember("owner", owner, allocator);
            value.AddMember("civil_code", civil_code, allocator);
            value.AddMember("address", address, allocator);
            value.AddMember("parental", parental, allocator);
            value.AddMember("parent_id", parent_id, allocator);
            value.AddMember("register_way", register_way, allocator);
            value.AddMember("safety_way", safety_way, allocator);
            value.AddMember("secrecy", secrecy, allocator);
            value.AddMember("status", status, allocator);
            arr_client_info.PushBack(value, allocator);
        }
        value.AddMember("channels", arr_client_info, allocator);
        arr_device.PushBack(value, allocator);
    }
    doc.AddMember("device_list", arr_device, allocator);
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    doc.Accept(writer);
    device_list = buffer.GetString();
    CLOGI(BLUE, "device_list:%s", device_list.c_str());

    resp->json["code"] = 0; // 鉴权成功
    resp->json["data"]["action"] = "query_device_list";
    resp->json["msg"] = "success";
    return resp->String(device_list);
}

int XHttpServer::query_device_library(HttpRequest* req, HttpResponse* resp)
{
    return resp->String(query_device_library__(req, resp));
}

std::string XHttpServer::query_device_library__(HttpRequest* req, HttpResponse* resp)
{
    std::string device_id = req->GetParam("device_id");
    if (device_id.empty()) {
        return get_simple_info(400, "错误的device_id");
    }
    std::string start_time = req->GetParam("start_time");
    std::string end_time = req->GetParam("end_time");
    auto client_ptr = Server::instance()->FindClientEx(device_id);
    if (!client_ptr) {
        return get_simple_info(101, "can not find the device client");
    }
    CLOGI(YELLOW, "start_time:%s end_time:%s", start_time.c_str(), end_time.c_str());

    auto record_infos = Server::instance()->GetRecordInfo(device_id);
    decltype(record_infos) valid_records;

    rapidjson::Document doc(rapidjson::kObjectType);    // doc.SetObject();
    rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();
    
    rapidjson::Value arr_record_info(rapidjson::kArrayType);
    for (const auto& record_info : record_infos) {
        if (!start_time.empty() && record_info->start_time.compare(start_time) < 0) {
            break;
        }
        if (!end_time.empty() && record_info->end_time.compare(end_time) > 0) {
            break;
        }
        valid_records.emplace_back(record_info);

        rapidjson::Value value(rapidjson::kObjectType);
        rapidjson::Value device_id(rapidjson::kStringType);
        device_id.SetString(record_info->device_id.c_str(), allocator);
        rapidjson::Value name(rapidjson::kStringType);
        name.SetString(record_info->name.c_str(), allocator);
        rapidjson::Value file_path(rapidjson::kStringType);
        file_path.SetString(record_info->file_path.c_str(), allocator);
        rapidjson::Value address(rapidjson::kStringType);
        address.SetString(record_info->address.c_str(), allocator);
        rapidjson::Value start_time(rapidjson::kStringType);
        start_time.SetString(record_info->start_time.c_str(), allocator);
        rapidjson::Value end_time(rapidjson::kStringType);
        end_time.SetString(record_info->end_time.c_str(), allocator);
        rapidjson::Value secrecy(rapidjson::kNumberType);
        secrecy.SetInt(record_info->secrecy);
        rapidjson::Value type(rapidjson::kStringType);
        type.SetString(record_info->type.c_str(), allocator);

        value.AddMember("device_id", device_id, allocator);
        value.AddMember("name", name, allocator);
        value.AddMember("file_path", file_path, allocator);
        value.AddMember("address", address, allocator);
        value.AddMember("start_time", start_time, allocator);
        value.AddMember("end_time", end_time, allocator);
        value.AddMember("secrecy", secrecy, allocator);
        value.AddMember("type", type, allocator);
        arr_record_info.PushBack(value, allocator);
    }
    doc.AddMember("record_list", arr_record_info, allocator);
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    doc.Accept(writer);
    auto record_list_str = buffer.GetString();
    return record_list_str;
}

int XHttpServer::refresh_record_history__(HttpRequest* req, HttpResponse* resp)
{
    std::string device_id = req->GetParam("device_id");
    if (device_id.empty()) {
        return -1;
    }
    std::string start_time = req->GetParam("start_time");
    if (start_time.empty()) {
        return -2;
    }
    std::string end_time = req->GetParam("end_time");
    if (end_time.empty()) {
        return -3;
    }
    auto client_ptr = Server::instance()->FindClientEx(device_id);
    if (!client_ptr) {
        return -4;
    }
    CLOGI(YELLOW, "start_time:%s end_time:%s", start_time.c_str(), end_time.c_str());
    auto req_ptr = std::make_shared<ClientRequest>();

    {
        char sz_start_time[32] = {0};
        char sz_end_time[32] = {0};
        int year, month, day, hour, min, sec;
        sscanf(start_time.c_str(), "%d-%d-%d %d:%d:%d", &year, &month, &day, &hour, &min, &sec);
        sprintf(sz_start_time, "%d-%02d-%02dT%02d:%02d:%02d", year, month, day, hour, min, sec);
        sscanf(end_time.c_str(), "%d-%d-%d %d:%d:%d", &year, &month, &day, &hour, &min, &sec);
        sprintf(sz_end_time, "%d-%02d-%02dT%02d:%02d:%02d", year, month, day, hour, min, sec);
        XmlQueryLibraryParamPtr params_ptr = std::make_shared<XmlQueryLibraryParam>();
        client_ptr->param_ptr = params_ptr;
        params_ptr->device_id = device_id;
        params_ptr->cmd = "RecordInfo";
        params_ptr->query_type = kXmlQueryFileLibrary;
        params_ptr->start_time = sz_start_time;
        params_ptr->end_time = sz_end_time;
        CLOGI(YELLOW, "params_ptr->start_time:%s params_ptr->end_time:%s", params_ptr->start_time.c_str(), params_ptr->end_time.c_str());
    }
    client_ptr->real_device_id = device_id;
    req_ptr->client_ptr = client_ptr;
    req_ptr->req_type = kRequestTypeRefreshLibrary;

    Server::instance()->RemoveRecordInfo(device_id);
    Server::instance()->AddRequest(req_ptr);
    return 0;
}

int XHttpServer::refresh_device_library(HttpRequest* req, HttpResponse* resp)
{
    int ret = refresh_record_history__(req, resp);
    resp->json["code"] = ret;
    resp->json["data"]["action"] = "refresh_device_list";
    resp->json["msg"] = (ret < 0 ? "failed" : "success");
    return kHttpOK;
}

int XHttpServer::refresh_device_library_async(const HttpContextPtr& context)
{
    if (refresh_record_history__(context->request.get(), context->response.get()) < 0) {
        return context->response->String("refresh record history error!");
    }
    hv::async([context, this] () {
        Server::instance()->WaitHistory();
        //std::this_thread::sleep_for(std::chrono::seconds(10));
        std::string history_video_list = query_device_library__(context->request.get(), context->response.get());
        context->send(history_video_list, context->type());
    });
    return 0;
}

int XHttpServer::get_snap(HttpRequest* req, HttpResponse* resp)
{
    std::string ssrc = req->GetParam("ssrc");   // 16进制
    if (ssrc.empty()) {
        return resp->String(get_simple_info(400, "can not find param ssrc!"));
    }
    std::string media_server = Server::instance()->GetServerInfo().rtp_ip;
    std::stringstream ss;
    ss << "rtsp://" << media_server << "/rtp/" << ssrc;
    std::string rtsp_url = ss.str();
    ss.str("");
    ss << "http://" << media_server << "/index/api/getSnap"
    << "?url=" << rtsp_url
    << "&timeout_sec=" << 10
    << "&expire_sec=" << 30
    << "&secret=" << "Lsb4XJqAdK0QLVErbKEvBBGrSDJ3lexS";
    std::string snap_url = ss.str();
    ss.str("");
    ss << "./imgs/" << Xzm::util::Timer::instance()->GetCurrentTime() << "_" << ssrc << ".jpg";
    std::string filepath = ss.str();
    requests::downloadFile(snap_url.c_str(), filepath.c_str());
    return resp->File(filepath.c_str());
}

int XHttpServer::start_rtsp_publish(HttpRequest* req, HttpResponse* resp)
{
    std::string device = req->GetParam("device");
    if (device.empty()) {
        return resp->String(get_simple_info(400, "can not find param device!"));
    }
    auto client_ptr = Server::instance()->FindClientEx(device);
    if (!client_ptr) {
        return resp->String(get_simple_info(101, "can not find the device client"));
    }
    auto req_ptr = std::make_shared<ClientRequest>();
    req_ptr->client_ptr = client_ptr;
    client_ptr->real_device_id = device;
    auto s_info = Server::instance()->GetServerInfo();
    client_ptr->ssrc = Xzm::util::build_ssrc(true, s_info.realm);
    auto ssrc = Xzm::util::convert10to16(client_ptr->ssrc);
    client_ptr->rtsp_url = Xzm::util::get_rtsp_addr(s_info.rtp_ip, ssrc);
    req_ptr->req_type = kRequestTypeInvite;
    Server::instance()->AddRequest(req_ptr);
    CLOGI(RED, "seond invite request...............................");
    resp->json["code"] = 0; // 鉴权成功
    resp->json["data"]["device"] = device;
    resp->json["data"]["action"] = "start_rtsp_publish";
    resp->json["data"]["rtsp"] = client_ptr->rtsp_url;
    resp->json["msg"] = "success";
    return kHttpOK;
}

int XHttpServer::stop_rtsp_publish(HttpRequest* req, HttpResponse* resp)
{
    std::string device = req->GetParam("device");
    if (device.empty()) {
        return resp->String(get_simple_info(400, "can not find param device!"));
    }
    auto client_ptr = Server::instance()->FindClient(device);
    if (!client_ptr) {
        return resp->String(get_simple_info(101, "can not find the device client"));
    }
    auto req_ptr = std::make_shared<ClientRequest>();
    req_ptr->client_ptr = client_ptr;
    req_ptr->req_type = kRequestTypeCancel;
    Server::instance()->AddRequest(req_ptr);
    resp->json["code"] = 0; // 鉴权成功
    resp->json["data"]["device"] = device;
    resp->json["data"]["action"] = "stop_rtsp_publish";
    resp->json["msg"] = "success";
    return kHttpOK; // http调用成功
}

int XHttpServer::start_invite_talk(HttpRequest* req, HttpResponse* resp)
{
    std::string device = req->GetParam("device");
    if (device.empty()) {
        return resp->String(get_simple_info(400, "can not find param device!"));
    }
    auto client_ptr = Server::instance()->FindClient(device);
    if (!client_ptr) {
        return resp->String(get_simple_info(101, "can not find the device client"));
    }
    auto req_ptr = std::make_shared<ClientRequest>();
    req_ptr->client_ptr = client_ptr;
    req_ptr->req_type = kRequestTypeTalk;
    Server::instance()->AddRequest(req_ptr);
    resp->json["code"] = 0; // 鉴权成功
    resp->json["data"]["device"] = device;
    resp->json["data"]["action"] = "invite_talk";
    resp->json["msg"] = "success";
    return kHttpOK;
}

int XHttpServer::stop_talk(HttpRequest* req, HttpResponse* resp)
{
    return kHttpOK;
}

int XHttpServer::start_talk_broadcast(HttpRequest* req, HttpResponse* resp)
{
    std::string device_id = req->GetParam("device_id");
    if (device_id.empty()) {
        return resp->String(get_simple_info(400, "错误的device_id"));
    }
    auto client_ptr = Server::instance()->FindClientEx(device_id);
    if (!client_ptr) {
        return resp->String(get_simple_info(400, "找不到对应的设备信息"));
    }
    auto req_ptr = std::make_shared<ClientRequest>();
    req_ptr->client_ptr = client_ptr;
    req_ptr->req_type = kRequestTypeBroadcast;
    Server::instance()->AddRequest(req_ptr);
    resp->json["code"] = 0; // 鉴权成功
    resp->json["data"]["device"] = device_id;
    resp->json["data"]["action"] = "broadcast";
    resp->json["msg"] = "success";

    return kHttpOK;
}

int XHttpServer::start_playback(HttpRequest* req, HttpResponse* resp)
{
    std::string device_id = req->GetParam("device_id");
    if (device_id.empty()) {
        return resp->String(get_simple_info(400, "can not find param device!"));
    }
    auto client_ptr = Server::instance()->FindClientEx(device_id);
    if (!client_ptr) {
        return resp->String(get_simple_info(400, "can not find the device client"));
    }
    std::string start_time = req->GetParam("start_time");
    if (start_time.empty()) {
        return resp->String(get_simple_info(400, "start_time can not be null!"));
    }
    std::string end_time = req->GetParam("end_time");
    if (end_time.empty()) {
        return resp->String(get_simple_info(400, "end_time can not be null!"));
    }
    auto req_ptr = std::make_shared<ClientRequest>();
    client_ptr->real_device_id = device_id;
    req_ptr->client_ptr = client_ptr;
    req_ptr->req_type = kRequestTypePlayback;
    auto param_ptr = std::make_shared<RequestParamQueryHistory>();
    req_ptr->param_ptr = param_ptr;
    try {
        param_ptr->start_time = std::stoi(start_time);
        param_ptr->end_time = std::stoi(end_time);
    } catch (std::exception& e) {
        CLOGE(RED, "%s", e.what());
        return resp->String(get_simple_info(400, e.what()));
    }
    auto s_info = Server::instance()->GetServerInfo();
    client_ptr->ssrc = Xzm::util::build_ssrc(false, s_info.realm);
    auto ssrc = Xzm::util::convert10to16(client_ptr->ssrc);
    client_ptr->rtsp_url = Xzm::util::get_rtsp_addr(s_info.rtp_ip, ssrc);
    Server::instance()->AddRequest(req_ptr);
    resp->json["code"] = 0; // 鉴权成功
    resp->json["data"]["device"] = device_id;
    resp->json["data"]["action"] = "playback";
    resp->json["data"]["rtsp"] = client_ptr->rtsp_url;
    resp->json["msg"] = "success";
    return kHttpOK;
}

int XHttpServer::fast_forward_playback(HttpRequest* req, HttpResponse* resp)
{
    std::string device_id = req->GetParam("device_id");
    if (device_id.empty()) {
        return resp->String(get_simple_info(400, "can not find param device_id!"));
    }
    std::string ssrc = req->GetParam("ssrc");
    if (ssrc.empty()) {
        return resp->String(get_simple_info(400, "can not find param ssrc!"));
    }
    std::string scale = req->GetParam("scale");
    if (scale.empty()) {
        return resp->String(get_simple_info(400, "can not find param scale!"));
    }
    auto client_ptr = Server::instance()->FindClientEx(device_id);
    if (!client_ptr) {
        return resp->String(get_simple_info(400, "can not find the device client"));
    }
    auto req_ptr = std::make_shared<ClientRequest>();
    client_ptr->real_device_id = device_id;
    req_ptr->client_ptr = client_ptr;
    req_ptr->req_type = kRequestTypeFastforwardPlayback;
    auto param_ptr = std::make_shared<RequestParamFastforward>();
    param_ptr->ssrc = ssrc;
    param_ptr->scale = scale;
    req_ptr->param_ptr = param_ptr;
    Server::instance()->AddRequest(req_ptr);
    resp->json["code"] = 0; // 鉴权成功
    resp->json["data"]["device"] = device_id;
    resp->json["data"]["action"] = "fast_forward_playback";
    resp->json["msg"] = "success";
    return kHttpOK;
}

int XHttpServer::on_publish(HttpRequest* req, HttpResponse* resp)
{
    CLOGI(CYAN, "http on publish!!!-------------------------------------------------------------------");
    std::cout << BLUE << req->Dump(true, true) << DEFAULT_COLOR << std::endl;
    int port;
    std::string app, id, ip, params, schema, stream, vhost, media_server_id;
    //resp->http_cb;
    auto json = req->GetJson();
    HV_JSON_GET_STRING(json, app, "app"); // 流应用名
    HV_JSON_GET_STRING(json, ip, "ip"); // 国标设备ip
    HV_JSON_GET_STRING(json, id, "id"); // TCP链接唯一ID
    HV_JSON_GET_STRING(json, params, "params"); // 播放url参数
    HV_JSON_GET_STRING(json, schema, "schema"); // 播放的协议，可能是rtsp、rtmp、http
    HV_JSON_GET_STRING(json, stream, "stream"); // 流ID
    HV_JSON_GET_STRING(json, vhost, "vhost"); // 流虚拟主机
    HV_JSON_GET_STRING(json, media_server_id, "mediaServerId"); // 服务器id,通过配置文件设置
    HV_JSON_GET_INT(json, port, "port"); // 播放器端口号

    std::stringstream ss;
    ss << "rtsp://" << Server::instance()->GetServerInfo().ip << "/rtp/" << stream;
    std::string rtsp_url = ss.str();
    std::cout << RED
    << "app:" << app << std::endl
    << "id:" << id << std::endl
    << "ip:" << ip << std::endl
    << "params:" << params << std::endl
    << "port:" << port << std::endl
    << "schema:" << schema << std::endl
    << "stream:" << stream << std::endl
    << "vhost:" << vhost << std::endl
    << "media_server_id:" << media_server_id << std::endl
    << "rtsp_url:" << rtsp_url << std::endl << DEFAULT_COLOR;
    resp->json["code"] = 0; // 鉴权成功
    resp->json["msg"] = "success";
    return kHttpOK; // http调用成功
}

/*
{
        "app" : "rtp",
        "hook_index" : 16,
        "id" : "18-42",
        "ip" : "10.23.132.77",
        "mediaServerId" : "your_server_id",
        "params" : "",
        "port" : 3119,
        "schema" : "rtsp",
        "stream" : "0BEBE618",
        "vhost" : "__defaultVhost__"
}
*/
int XHttpServer::on_play(HttpRequest* req, HttpResponse* resp)
{
    CLOGI(RED, "------------------------------------------http on play---------------------------------");
    std::cout << RED << req->Dump(true, true) << DEFAULT_COLOR << std::endl;

    int port;
    std::string app, id, ip, params, schema, stream, vhost, media_server_id;
    auto json = req->GetJson();
    HV_JSON_GET_STRING(json, app, "app"); // 流应用名
    HV_JSON_GET_STRING(json, ip, "ip"); // 播放器ip
    HV_JSON_GET_STRING(json, id, "id"); // TCP链接唯一ID
    HV_JSON_GET_STRING(json, params, "params"); // 播放url参数
    HV_JSON_GET_STRING(json, schema, "schema"); // 播放的协议，可能是rtsp、rtmp、http
    HV_JSON_GET_STRING(json, stream, "stream"); // 流ID
    HV_JSON_GET_STRING(json, vhost, "vhost"); // 流虚拟主机
    HV_JSON_GET_STRING(json, media_server_id, "mediaServerId"); // 服务器id,通过配置文件设置
    HV_JSON_GET_INT(json, port, "port"); // 播放器端口号

    resp->json["code"] = 0; // 鉴权成功
    resp->json["msg"] = "success";
    return kHttpOK; // http调用成功
}
};