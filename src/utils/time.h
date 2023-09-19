/**
 * @file time.h
 * @author sleeping csleeping@163.com
 * @brief 时间类
 * @version 0.1
 * @date 2023-09-19
 * 
 * @copyright Copyright (c) 2023
 * 引用自:https://www.jianshu.com/p/a1c0a82efdba
 */

#pragma once

#include <chrono>
#include <ctime>
#include <string>
#include <cstring>
#include <cstdlib>
#include <iomanip>
#include <sstream>

namespace Xzm
{
// 日期时间格式化的常量字符串
extern const std::string TIME_FORMAT;
// 重命名system_clock名称空间
using system_clk = std::chrono::system_clock;
// 重命名time_point类型
using _time_point = std::chrono::time_point<system_clk>;

class Timer {
    public: 
        Timer(): m_begin(system_clk::now()) {}
        ~Timer() {}
        
        inline void reset() {
            m_begin = system_clk::now();
        }

        // 将时间点信息转换为字符串的函数
        bool to_string(const _time_point& t, const std::string& date_fmt, std::string& result);

        // 将字符串转换为time_point的函数
        void from_string(const std::string &src_str, const std::string& date_fmt, _time_point& out_t);
        // 默认输出毫秒
        int64_t elapsed() const;
        // 微秒
        int64_t elapsed_micro() const;
        // 纳秒
        int64_t elapsed_nano() const;
        // 秒
        int64_t elapsed_sec() const;
        // 分
        int64_t elapsed_min() const;
        // 时
        int64_t elapsed_hour() const;

    private:
        _time_point m_begin;
};
};