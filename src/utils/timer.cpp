#include "timer.h"
#include <iomanip>
#include <sstream>

namespace Xzm {
namespace util {

Timer::Timer()
{

}

Timer::~Timer()
{

}

std::string Timer::XGetCurrentTime()
{
    time_t t = time(nullptr);
    struct tm _tm;
#ifdef LINUX
    localtime_r(&t, &_tm);
#endif
#ifdef WIN32
    localtime_s(&_tm, &t);
#endif
    std::stringstream ss;
    ss << std::setw(4) << std::setfill('0') << _tm.tm_year + 1900 << "_"
    << std::setw(2) << std::setfill('0') << _tm.tm_mon + 1 << "_"
    << std::setw(2) << std::setfill('0') << _tm.tm_mday << "_"
    << std::setw(2) << std::setfill('0') << _tm.tm_hour << "_"
    << std::setw(2) << std::setfill('0') << _tm.tm_min << "_"
    << std::setw(2) << std::setfill('0') << _tm.tm_sec;
    return ss.str();
}

}
};