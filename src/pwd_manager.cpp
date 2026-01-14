#include "pwd_manager.h"
#include <Windows.h>
#include <iostream>
#include <NTSecAPI.h>
#include <ntstatus.h>
#pragma comment(lib, "Secur32.lib")
#pragma comment(lib, "Advapi32.lib")

PwdManager& PwdManager::Instance() {
	static PwdManager instance;
	return instance;
}

bool PwdManager::Verify(LPCWSTR username, LPCWSTR domain, LPCWSTR password)
{
    HANDLE hToken = NULL;

    // 使用 LogonUser 获取用户令牌
    if (!LogonUser(username, domain, password,
        LOGON32_LOGON_INTERACTIVE,  // 登录类型
        LOGON32_PROVIDER_DEFAULT,   // 登录提供者
        &hToken))
    {
        DWORD error = GetLastError();
        // 错误处理
        return false;
    }

    return true;
}

