#pragma once

#include "debug.h"
#include <windows.h>
#include <lm.h>
#include <iostream>

//enum WindowsLogonType {
//    LOGON_TYPE_UNKNOWN = 0,
//    LOGON_TYPE_INTERACTIVE = 2,        // 交互式登录（控制台）
//    LOGON_TYPE_NETWORK = 3,            // 网络登录
//    LOGON_TYPE_BATCH = 4,              // 批处理登录
//    LOGON_TYPE_SERVICE = 5,            // 服务登录
//    LOGON_TYPE_PROXY = 6,              // 代理登录
//    LOGON_TYPE_UNLOCK = 7,             // 工作站解锁
//    LOGON_TYPE_NETWORK_CLEARTEXT = 8,  // 网络明文登录
//    LOGON_TYPE_NEW_CREDENTIALS = 9,    // 新凭据登录
//    LOGON_TYPE_REMOTE_INTERACTIVE = 10,// 远程交互登录（RDP/Terminal Services）
//    LOGON_TYPE_CACHED_INTERACTIVE = 11,// 缓存交互登录
//    LOGON_TYPE_CACHED_REMOTE_INTERACTIVE = 12, // 缓存远程交互
//    LOGON_TYPE_CACHED_UNLOCK = 13      // 缓存解锁
//};
//
// 将登录类型转换为字符串

class PwdManager
{
public:
    static PwdManager& Instance();
    bool Verify(LPCWSTR username, LPCWSTR domain, LPCWSTR password);

private:
    PwdManager() {};
};
