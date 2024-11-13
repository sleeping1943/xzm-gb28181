#include "xzm_defines.h"
#include <memory>

namespace Xzm {

XmlQueryParam::XmlQueryParam() {}

// XmlQueryParam::XmlQueryParam(XmlQueryType _q_type, const std::string& _cmd,
// uint64_t _sn, const std::string& _device_id)
//     :query_type(_q_type), cmd(_cmd),sn(_sn),device_id(_device_id)
//{

//}

XmlQueryParam::~XmlQueryParam() {}

XmlQueryLibraryParam::XmlQueryLibraryParam() : XmlQueryParam() {}

XmlQueryLibraryParam::~XmlQueryLibraryParam() {}

std::string XmlQueryInfo::BuildMsg(const XmlQueryParamPtr &msg_in) {
  std::stringstream ss;
  ss << "<?xml version=\"1.0\"?>\r\n"
     << "<Query>\r\n"
     << "<CmdType>" << msg_in->cmd << "</CmdType>\r\n"
     << "<SN>" << msg_in->sn << "</SN>\r\n"
     << "<DeviceID>" << msg_in->device_id << "</DeviceID>\r\n"
     << ExtroXmlQueryParamfo(msg_in) << "</Query>\r\n";
  return ss.str();
}

std::string XmlQueryInfo::ExtroXmlQueryParamfo(const XmlQueryParamPtr &msg_in) {
  return "";
}

XmlQueryLibraryInfo::XmlQueryLibraryInfo() {}

XmlQueryLibraryInfo::~XmlQueryLibraryInfo() {}

std::string
XmlQueryLibraryInfo::ExtroXmlQueryParamfo(const XmlQueryParamPtr &msg_in) {
  std::string xml_str;
  XmlQueryLibraryParamPtr param_ptr =
      std::dynamic_pointer_cast<XmlQueryLibraryParam>(msg_in);
  if (!param_ptr) {
    return xml_str;
  }
  std::stringstream ss;
  ss << "<StartTime>" << param_ptr->start_time << "</StartTime>\r\n"
     << "<EndTime>" << param_ptr->end_time << "</EndTime>\r\n"
     << "<Secrecy>" << param_ptr->secrecy << "</Secrecy>\r\n"
     << "<StreamNumber>" << param_ptr->stream_number << "</StreamNumber>\r\n"
     << "<AlarmMethod>" << param_ptr->alarm_method << "</AlarmMethod>\r\n"
     << "<AlarmType>" << param_ptr->alarm_type << "</AlarmType>\r\n";
  if (!param_ptr->file_path.empty()) {
    ss << "<FilePath>" << param_ptr->file_path << "</FilePath>\r\n";
  }
  if (!param_ptr->address.empty()) {
    ss << "<Address>" << param_ptr->address << "</Address>\r\n";
  }
  if (!param_ptr->type.empty()) {
    ss << "<Type>" << param_ptr->type << "</Type>\r\n";
  }
  if (!param_ptr->recorder_id.empty()) {
    ss << "<RecorderID>" << param_ptr->recorder_id << "</RecorderID>\r\n";
  }
  xml_str = ss.str();
  return xml_str;
}
}; // namespace Xzm
