/**
 * @file msg_builder.h
 * @author sleeping csleeping@163.com
 * @brief 用于构造xml消息体
 * @version 0.1
 * @date 2023-09-15
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#include "../utils/singleton.h"
#include "../xzm_defines.h"

namespace Xzm
{
    class MsgBuilder : public util::Singleton<MsgBuilder>
    {
        friend class Singleton;
#ifdef LINUX
    private:
#endif
#ifdef WIN32
    public:
#endif
        MsgBuilder();

    public:
        ~MsgBuilder();
        std::string BuildMsg(XmlQueryParamPtr params_ptr);

    };
};