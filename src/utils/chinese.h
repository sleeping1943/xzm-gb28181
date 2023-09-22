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

namespace Xzm {
namespace util {
class Chinese : public Singleton<Chinese>
{
public:
	static std::string Utf8ToAnsi(const std::string& str);
	static std::string AnsiToUtf8(const std::string& str);


private:
	static std::string UnicodeToUtf8(const std::wstring& wstr);
	static std::wstring Utf8ToUnicode(const std::string& str);
	static std::string UnicodeToAnsi(const std::wstring& wstr);
	static std::wstring AnsiToUnicode(const std::string& str);

private:

};
}};