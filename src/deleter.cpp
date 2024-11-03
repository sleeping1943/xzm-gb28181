#include "deleter.h"
#include "./utils/log.h"
#include "fmt/format.h"
#include "http/http_server.h"
#include <boost/filesystem/directory.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <chrono>
#include <functional>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <sstream>
#include <stdio.h>
#include <thread>

using boost::filesystem::directory_iterator;
using boost::filesystem::path;

namespace Xzm {

XDeleter::XDeleter() {}

XDeleter::~XDeleter() {}

void XDeleter::Start() {
  is_stop = false;
  exe_path_ =
      boost::filesystem::initial_path<boost::filesystem::path>().string();
  img_path_ = exe_path_ + "/imgs";
  thread_ = std::thread(std::bind(&XDeleter::Run, this));
}

void XDeleter::Stop() {
  is_stop = true;
  if (thread_.joinable()) {
    thread_.join();
    LOG(INFO) << "quit deleter thread";
  }
}
std::string XDeleter::delete_time(int interval) {
  time_t t = time(nullptr);
  t -= interval;
  struct tm *_tm = localtime(&t);
  std::stringstream ss;
  ss << std::setw(4) << std::setfill('0') << _tm->tm_year + 1900 << "_"
     << std::setw(2) << std::setfill('0') << _tm->tm_mon + 1 << "_"
     << std::setw(2) << std::setfill('0') << _tm->tm_mday << "_" << std::setw(2)
     << std::setfill('0') << _tm->tm_hour << "_" << std::setw(2)
     << std::setfill('0') << _tm->tm_min << "_" << std::setw(2)
     << std::setfill('0') << _tm->tm_sec;
  return ss.str();
}

void XDeleter::Run() {
  while (true) {
    if (is_stop) {
      break;
    }
    path data_path(img_path_);
    directory_iterator iter_end;
    directory_iterator iter_begin(data_path);
    unsigned int cache_time = XHttpServer::instance()->GetSnapCacheTime();
    std::string str_delete_flag = delete_time(cache_time) + "_xxxxxxxx.jpg";
    std::vector<std::string> vec_file_delete;
    for (; iter_begin != iter_end; ++iter_begin) {
      if (boost::filesystem::is_regular_file(*iter_begin)) {
        std::string file_name = iter_begin->path().filename().string();
        if (file_name < str_delete_flag) {
          LOG(INFO) << fmt::format("{}<{} ready to delete it!{}", file_name,
                                   str_delete_flag,
                                   iter_begin->path().string());
          vec_file_delete.emplace_back(iter_begin->path().string());
        }
      }
    }
    for (const auto &file_path : vec_file_delete) {
      std::remove(file_path.c_str());
    }
    std::this_thread::sleep_for(std::chrono::seconds(5));
  }
  LOG(INFO) << "XDeleter ready to exit";
}
}; // namespace Xzm
