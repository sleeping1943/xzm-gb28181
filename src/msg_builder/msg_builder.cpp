#include "msg_builder.h"
#include "../utils/log.h"
#include "fmt/format.h"

namespace Xzm {

MsgBuilder::MsgBuilder() {}

MsgBuilder::~MsgBuilder() {}

std::string MsgBuilder::BuildMsg(XmlQueryParamPtr params_ptr) {
  std::string msg;
  XmlQueryInfoPtr info_ptr = nullptr;
  switch (params_ptr->query_type) {
  case kXmlQueryFileLibrary:
    info_ptr = std::make_shared<XmlQueryLibraryInfo>();
    break;
  default:
    break;
  }
  if (info_ptr) {
    msg = info_ptr->BuildMsg(params_ptr);
  }
  LOG(INFO) << fmt::format("BuildMsg:%s", msg.c_str());
  return msg;
}
}; // namespace Xzm
