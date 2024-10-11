#include <boost/date_time/posix_time/posix_time_duration.hpp>
#include <boost/thread/thread_time.hpp>
#include <chrono>
#include <thread>
#include "server.h"
#include "utils/helper.h"
#include <signal.h>
#include "utils/log.h"
#include "http/http_server.h"
#include "xzm_defines.h"
#include "deleter.h"
#include <boost/interprocess/sync/interprocess_semaphore.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

const static std::string kConfPath = "./conf/config.json";

boost::interprocess::interprocess_semaphore semaphore(0);

void quit_server(int)
{
    Xzm::Server::is_server_quit.store(true);
    CLOGI(RED, "ready to quit server!!");
    semaphore.post();
}

int main(int argc, char** argv)
{
    setlocale(LC_CTYPE, "");    // 使ansi编码有效,用于utf8和ansi转码,但是所有lib都会受影响,有风险
    signal(SIGINT, quit_server);
    Xzm::Server::instance()->Test();
    std::string content;
#ifdef WIN32
        WSADATA wsa;
        if (WSAStartup(MAKEWORD(2, 2), &wsa) !=0 ) {
            LOGE("init network error!");
            return -1;
        }
#endif

    // 启动sip服务
    if (!Xzm::Server::instance()->Init(kConfPath)) {
        LOGE("init server error!");
        return -1;
    }
    if (!Xzm::Server::instance()->Start()) {
        LOGE("start server error!");
        return -1;
    }
    CLOGI(BLUE, "s_info_:%s", Xzm::Server::instance()->GetServerInfo().str().c_str());
    if (!Xzm::XHttpServer::instance()->Init(kConfPath)) {
        LOGE("init http_server error!");
        return -2;
    }
    // 启动http服务
    if (!Xzm::XHttpServer::instance()->Start()) {
        LOGE("start http_server error!");
        return -2;
    }
    // 启动删除文件线程
    Xzm::XDeleter::instance()->Start();
    // 等待服务退出
    //unsigned int interval = 30;
    try {
    unsigned int interval = 5;
    boost::system_time wait_time = boost::get_system_time() + boost::posix_time::milliseconds(interval * 1000);
    while (!Xzm::Server::is_server_quit && !Xzm::Server::is_client_all_quit) {
        CLOGI(YELLOW, "wait for server quit[%ds interval]...", interval);
        //std::this_thread::sleep_for(std::chrono::milliseconds(interval * 1000));
        semaphore.timed_wait(wait_time);
        wait_time = boost::get_system_time() + boost::posix_time::milliseconds(interval * 1000);
    }
    } catch (std::exception& e) {
        std::stringstream ss;
        ss << "there is a error:" << e.what();
        LOGE("%s", ss.str().c_str());
    }
    // 关闭删除文件线程
    Xzm::XDeleter::instance()->Stop();
    // 关闭sip和http服务
    Xzm::Server::instance()->Stop();
    Xzm::XHttpServer::instance()->Stop();
    LOG_RED("quit server gracefully!!");
    return 0;
}
