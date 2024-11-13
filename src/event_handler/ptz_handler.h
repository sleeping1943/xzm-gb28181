/**
 * @file
 * @brief [发送摄像头控制命令]
 *
 *
 * 2024-11-06
 * author:sleeping
 */

#pragma once

#include "../xzm_defines.h"
#include "handler.h"

using Xzm::ClientPtr;
using Xzm::ClientRequestPtr;

/**
 * @class PtzHandler
 * @brief [Ptz控制类]
 *
 */
class PtzHandler : public Xzm::Handler {
public:
  PtzHandler();
  ~PtzHandler();

  /**
   * @brief [无应答发送控制命令]
   * 包括:远程启动、强制关键帧、拉框放大、拉框缩小等
   *
   * @param sip_context [sip上下文]
   * @param ptr [客户端请求参数]
   */
  void request_ptz_without_ack(eXosip_t *sip_context, ClientRequestPtr ptr);

  /**
   * @brief [有应答发送控制命令]
   * 包括:录像控制、报警布防/撤防、报警复位、看守位控制、设备配置命令等
   * @param sip_context [sip上下文]
   * @param ptr [客户端请求参数]
   */
  void request_ptz_with_ack(eXosip_t *sip_context, ClientRequestPtr ptr);

private:
  std::string generate_ptz_cmd(ClientRequestPtr);
  std::string generate_record_cmd(ClientRequestPtr);
};
using PtzHandlerPtr = std::shared_ptr<PtzHandler>;
