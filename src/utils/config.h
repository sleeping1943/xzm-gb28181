/**
 * @file config.h
 * @author sleeping (csleeping@163.com)
 * @brief 整个项目的配置管理
 * @version 0.1
 * @date 2024-10-14
 * 
 * @copyright Copyright (c) 2024
 * 
 */
#pragma once
#include "singleton.h"
#include "../xzm_defines.h"

class ConfigManager : public Xzm::util::Singleton<ConfigManager>
{
public:
    ConfigManager();
    ~ConfigManager();

    bool Parse(const std::string&);

    Xzm::ServerInfo GetServerInfo();
    Xzm::MediaServerInfo GetMediaServerInfo();
private:
    Xzm::ServerInfo s_info_;
    Xzm::MediaServerInfo media_server_info_;
};

#define gConfigPtr ConfigManager::instance()
#define gServerInfo ConfigManager::instance()->GetServerInfo()
#define gMediaServerInfo ConfigManager::instance()->GetMediaServerInfo()
