#include "deleter.h"
#include "easylogging/easylogging++.h"
#include "fmt/format.h"
#include "http/http_server.h"
#include "server.h"
#include "utils/config.h"
#include "utils/helper.h"
#include "utils/log.h"
#include "xzm_defines.h"
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/posix_time/posix_time_duration.hpp>
#include <boost/interprocess/sync/interprocess_semaphore.hpp>
#include <boost/thread/thread_time.hpp>
#include <signal.h>

INITIALIZE_EASYLOGGINGPP

const static std::string kConfPath = "./conf/config.json";

boost::interprocess::interprocess_semaphore semaphore(0);

void quit_server(int) {
  Xzm::Server::is_server_quit.store(true);
  LOG(INFO) << "ready to quit server!!";
  semaphore.post();
}

int main(int argc, char **argv) {

  // 日志配置
  el::Loggers::addFlag(el::LoggingFlag::StrictLogFileSizeCheck);
  el::Configurations conf("./log.conf");
  el::Loggers::reconfigureAllLoggers(conf);

  setlocale(
      LC_CTYPE,
      ""); // 使ansi编码有效,用于utf8和ansi转码,但是所有lib都会受影响,有风险
  signal(SIGINT, quit_server);
  Xzm::Server::instance()->Test();
  std::string content;
#ifdef WIN32
  WSADATA wsa;
  if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
    LOGE("init network error!");
    return -1;
  }
#endif
  Xzm::util::read_file(kConfPath, content);
  if (!gConfigPtr->Parse(content)) {
    LOG(ERROR) << fmt::format("parse config[{}] error", kConfPath.c_str());
    return -1;
  }
  // 启动sip服务
  if (!Xzm::Server::instance()->Init(kConfPath)) {
    LOG(ERROR) << "init server error!";
    return -1;
  }
  if (!Xzm::Server::instance()->Start()) {
    LOG(INFO) << "start server error!";
    return -1;
  }
  LOG(INFO) << "s_info_:%s", gServerInfo.str().c_str();
  if (!Xzm::XHttpServer::instance()->Init(kConfPath)) {
    LOG(ERROR) << "init http_server error!";
    return -2;
  }
  // 启动http服务
  if (!Xzm::XHttpServer::instance()->Start()) {
    LOG(ERROR) << "start http_server error!";
    return -2;
  }
  // 启动删除文件线程
  Xzm::XDeleter::instance()->Start();
  // 等待服务退出
  // unsigned int interval = 30;
  try {
    unsigned int interval = 5;
    boost::system_time wait_time =
        boost::get_system_time() +
        boost::posix_time::milliseconds(interval * 1000);
    while (!Xzm::Server::is_server_quit && !Xzm::Server::is_client_all_quit) {
      LOG(INFO) << fmt::format("wait for server quit[{}s interval]...",
                               interval);
      // std::this_thread::sleep_for(std::chrono::milliseconds(interval *
      // 1000));
      semaphore.timed_wait(wait_time);
      wait_time = boost::get_system_time() +
                  boost::posix_time::milliseconds(interval * 1000);
    }
  } catch (std::exception &e) {
    LOG(ERROR) << fmt::format("this is an error:{}", e.what());
  }
  // 关闭删除文件线程
  LOG(INFO) << "ready to quit thread[deleter]";
  Xzm::XDeleter::instance()->Stop();
  LOG(INFO) << "already quit thread[deleter]";
  // 关闭sip和http服务
  LOG(INFO) << "ready to quit thread[SipServer]";
  Xzm::Server::instance()->Stop();
  LOG(INFO) << "already quit thread[SipServer]";
  LOG(INFO) << "ready to quit thread[HttpServer]";
  Xzm::XHttpServer::instance()->Stop();
  LOG(INFO) << "already quit thread[HttpServer]";
  LOG(INFO) << "quit server gracefully!!";
  return 0;
}
