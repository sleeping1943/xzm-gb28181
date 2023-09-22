/**
 * @file chinese.h
 * @author sleeping csleeping@163.com
 * @brief gbk,utf转码,使用unicode中转
 * @version 0.1
 * @date 2023-09-22
 * 
 * @copyright Copyright (c) 2023
 * 
 */
 #pragma once
#include <codecvt>
#include <string>
#include "singleton.h"
#include <iconv.h>
#include <vector>
#include <string.h>

namespace Xzm {
namespace util {
class Chinese : public Singleton<Chinese>
{
public:
    std::string GBKToUTF8(const std::string& strGBK);
    std::string UTFtoGBK(const char* utf8);
private:
    int code_convert(const char *from_charset, const char *to_charset, char *inbuf, size_t inlen, char *outbuf, size_t outlen);
    int u2g(char *inbuf, size_t inlen, char *outbuf, size_t outlen);
    int g2u(char *inbuf, size_t inlen, char *outbuf, size_t outlen);
};
}};