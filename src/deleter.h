/**
 * @file deleter.h
 * @author sleeping csleeping@163.com
 * @brief 删除截图的本地图片
 * @version 0.1
 * @date 2023-09-27
 * 
 * @copyright Copyright (c) 2023
 * 
 */
 #include "utils/singleton.h"
#include <queue>
 #include <thread>
 #include <atomic>

namespace Xzm
{
class XDeleter : public util::Singleton<XDeleter>
{
public:
    XDeleter();
    ~XDeleter();

    void Start();
    void Stop();
    void Run();

    std::string delete_time(int interval);

private:
    std::thread thread_;
    std::atomic_bool is_stop;
    std::string exe_path_;
    std::string img_path_;
};
};