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
#include "singleton.h"
#include <string>

namespace Xzm {
namespace util {

class Timer : public Singleton<Timer>
{
public:
    Timer();
    ~Timer();

    std::string GetCurrentTime();
};
}
};