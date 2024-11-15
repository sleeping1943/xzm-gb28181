/**
 * @file time.h
 * @author sleeping csleeping@163.com
 * @brief 时间类
 * @version 0.1
 * @date 2023-09-19
 *
 * @copyright Copyright (c) 2023
 */

#pragma once
#include <time.h>

#include <string>

#include "singleton.h"

namespace Xzm {
namespace util {

class Timer : public Singleton<Timer>
{
public:
    Timer();
    ~Timer();

    /**
     * @brief 当前时间字符串 格式:2023-01-03_23:13:01
     */
    std::string XGetCurrentTime();
    /**
     * @brief 获取当前时间字符串 格式:2023/03/12 23:04:59
     */
    std::string GetCurrentTimeStr(time_t t = 0);
};
}  // namespace util
};  // namespace Xzm
