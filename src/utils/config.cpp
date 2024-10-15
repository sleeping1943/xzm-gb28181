#include "config.h"
#include "json_helper.h"


ConfigManager::ConfigManager()
{

}

ConfigManager::~ConfigManager()
{

}

bool ConfigManager::Parse(const std::string& json_str)
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
    JSON_VALUE_REQUIRE_STRING(media_server_config, "secret", media_server_info_.secret);
    JSON_VALUE_REQUIRE_INT(media_server_config, "rtpPort", media_server_info_.rtp_port);
    JSON_VALUE_REQUIRE_INT(media_server_config, "rtp_proxy_port_min", media_server_info_.rtp_proxy_port_min);
    JSON_VALUE_REQUIRE_INT(media_server_config, "rtp_proxy_port_max", media_server_info_.rtp_proxy_port_max);
    return true;
}

Xzm::ServerInfo ConfigManager::GetServerInfo()
{
    return s_info_;
}

Xzm::MediaServerInfo ConfigManager::GetMediaServerInfo()
{
    return media_server_info_;
}