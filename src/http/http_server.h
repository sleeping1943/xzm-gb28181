/**
 * @file http_server.h
 * @author sleeping (csleeping@163.com)
 * @brief 一个简单的基于libhv的http服务器
 * @version 0.1
 * @date 2023-08-28
 *
 * @copyright Copyright (c) 2023
 *
 */
#pragma once
#include "../utils/singleton.h"
#include "../xzm_defines.h"
#include "hv/HttpServer.h"
#include <atomic>
#include <hv/HttpContext.h>
#include <thread>

#define GERNERATE_ERROR_INFO(err_code, msg)                                    \
  "{\"code\":err_code, msg:"                                                   \
  "}"

namespace Xzm {

/**
 * @class XHttpServer
 * @brief [http服务]
 *
 */
/**
 * @class XHttpServer
 * @brief [TODO:description]
 *
 */
class XHttpServer : public util::Singleton<XHttpServer> {
  friend class Singleton;

public:
  ~XHttpServer();
  bool Init(const std::string &conf_path);
  bool Start();
  bool Stop();
  bool Run();

  inline unsigned int GetSnapCacheTime() { return s_info_.snap_cache_time; }

#ifdef LINUX
private:
#endif
#ifdef LINUX
public:
#endif
  XHttpServer();

private:
  /**
   * @brief 根据device_id扫描设备信息
   *
   * @param req
   * @param resp
   * @return int
   */
  int scan_device_list(HttpRequest *req, HttpResponse *resp);

  /**
   * @brief 查询已注册设备信息
   *
   * @param req
   * @param resp
   * @return int
   */
  int query_device_list(HttpRequest *req, HttpResponse *resp);

  /**
   * @brief 查询设备历史记录信息
   *
   * @param req
   * @param resp
   * @return int
   */
  int query_device_library(HttpRequest *req, HttpResponse *resp);

  std::string query_device_library__(HttpRequest *req, HttpResponse *resp);

  int refresh_record_history__(HttpRequest *req, HttpResponse *resp);
  /**
   * @brief 刷新设备历史记录信息
   *
   * @param req
   * @param resp
   * @return int
   */
  int refresh_device_library(HttpRequest *req, HttpResponse *resp);

  /* 异步处理设备历史录像信息的查询 */
  int refresh_device_library_async(const HttpContextPtr &context);

  /* 获取截图 */
  int get_snap(HttpRequest *req, HttpResponse *resp);

  /**
   * @brief 请求指定设备开启推流
   *
   * @param req
   * @param resp
   * @return int
   */
  int start_rtsp_publish(HttpRequest *req, HttpResponse *resp);

  /**
   * @brief 使指定设备结束推流
   *
   * @param req
   * @param resp
   * @return int
   */
  int stop_rtsp_publish(HttpRequest *req, HttpResponse *resp);

  /**
   * @brief 指定设备开始对讲
   *
   * @param req
   * @param resp
   * @return int
   */

  int start_invite_talk(HttpRequest *req, HttpResponse *resp);
  /**
   * @brief 指定设备结束对讲
   *
   * @param req
   * @param resp
   * @return int
   */
  int stop_talk(HttpRequest *req, HttpResponse *resp);

  /**
   * @brief 广播对讲消息
   *
   * @param req
   * @param resp
   * @return int
   */
  int start_talk_broadcast(HttpRequest *req, HttpResponse *resp);

  /**
   * @brief 历史录像回放
   *
   * @param req
   * @param resp
   * @return int
   */
  int start_playback(HttpRequest *req, HttpResponse *resp);

  /**
   * @brief 快进历史录像
   *
   * @param req
   * @param resp
   * @return int
   */
  int fast_forward_playback(HttpRequest *req, HttpResponse *resp);

  /**
   * @brief 判断指定流是否存在
   *
   * @param req
   * @param resp
   * @return int
   */
  int check_stream(HttpRequest *req, HttpResponse *resp);

  /**
   * @brief [获取指定流信息]
   *
   * @param req [http请求参数]
   * @param resp [http响应]
   * @return [请求结果错误码 200-正常返回]
   */
  int get_rtp_info(HttpRequest *req, HttpResponse *resp);

  /**
   * @brief [发送摄像头控制命令]
   *
   * @param req [http请求]
   * @param resp [http响应]
   * @return [请求结果错误码 200-正常返回]
   */
  int send_camera_ptz_cmd(HttpRequest *req, HttpResponse *resp);

  /************************以下接口为zlmediakit回调函数****************************/
  /**
   * @brief 国标设备开始推送的回调及鉴权函数
   *
   * @param req
   * @param resp
   * @return int
   */
  int on_publish(HttpRequest *req, HttpResponse *resp);

  /**
   * @brief 客户端请求播放流的回调及鉴权函数
   *
   * @param req
   * @param resp
   * @return int
   */
  int on_play(HttpRequest *req, HttpResponse *resp);

  /**
   * @brief 当数据流注册或者注销时触发
   *
   * @param req
   * @param resp
   * @return int
   */
  int on_stream_changed(HttpRequest *req, HttpResponse *resp);

  inline std::string get_simple_info(int code, const std::string &msg) {
    std::stringstream ss;
    ss << "{"
       << "\"code\":" << code << ","
       << "\"msg\":" << msg << "}";
    return ss.str();
  }

private:
  hv::HttpService router;
  std::thread thread_;
  std::atomic_bool is_quit_;
  HttpServerInfo s_info_;
  http_server_t server_;
};
}; // namespace Xzm
