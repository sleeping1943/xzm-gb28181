#include "chinese.h"
#include <locale>
#include <iostream>

namespace Xzm {
namespace util {

std::string Chinese::GBKToUTF8(const std::string& strGBK)
{
    int length = strGBK.size()*2+1;
    char *temp = (char*)malloc(sizeof(char)*length);
    if(g2u((char*)strGBK.c_str(),strGBK.size(),temp,length) >= 0) {
        std::string str_result;
        str_result.append(temp);
        free(temp);
        return str_result;
    }else {
        free(temp);
        return "";
    }
}
 
std::string Chinese::UTFtoGBK(const char* utf8)
{
    int length = strlen(utf8);
    char *temp = (char*)malloc(sizeof(char)*length);
    if(u2g((char*)utf8,length,temp,length) >= 0) {
        std::string str_result;
        str_result.append(temp);
        free(temp);
        return str_result;
    }else {
        free(temp);
        return "";
    }
}

int Chinese::code_convert(const char *from_charset, const char *to_charset, char *inbuf, size_t inlen, char *outbuf, size_t outlen) 
{
    iconv_t cd;
    char **pin = &inbuf;
    char **pout = &outbuf;
 
    cd = iconv_open(to_charset, from_charset);
    if (cd == 0) {
        return -1;
    }
    memset(outbuf, 0, outlen);
    if (iconv(cd, pin, &inlen, pout, &outlen) == -1) {
        iconv_close(cd);
        return -1;
    }
    iconv_close(cd);
    return 0;
}
 
int Chinese::u2g(char *inbuf, size_t inlen, char *outbuf, size_t outlen) 
{
    return code_convert("utf-8", "gb2312", inbuf, inlen, outbuf, outlen);
}
 
int Chinese::g2u(char *inbuf, size_t inlen, char *outbuf, size_t outlen) 
{
    return code_convert("gb2312", "utf-8", inbuf, inlen, outbuf, outlen);
}

}};