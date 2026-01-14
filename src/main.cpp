#include "config.h"
#include "language.h"
#include "logger.h"
#include "pwd_manager.h"
#include <windows.h>
#include <shellapi.h>
#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <fstream>
#include <cstdlib>
#include <wincrypt.h>
#include <locale>
#include <cwchar>
#pragma comment(lib, "crypt32.lib")

namespace {
	std::string GetName() {
		std::string userName{ Logger::Instance().GetUserAtDomain() };
		std::string username;
		size_t backslashPos = userName.find('@');

		if (backslashPos != std::string::npos) {
			username = userName.substr(0, backslashPos);
			return username;
		}
		return "UNKNOWN";
	}

	std::string GetDomain() {
		std::string userName{ Logger::Instance().GetUserAtDomain() };
		std::string domain;
		size_t backslashPos = userName.find('@');

		if (backslashPos != std::string::npos) {
			domain = userName.substr(backslashPos + 1);
			return domain;
		}
		return "UNKNOWN";
	}

	std::wstring N2W(const std::string& narrowStr) {
		if (narrowStr.empty()) return L"";

		std::vector<wchar_t> buffer(narrowStr.size() * sizeof(wchar_t) + 1);
		const char* pSrc = narrowStr.c_str();
		wchar_t* pDest = buffer.data();
		std::mbstate_t state = std::mbstate_t();

		size_t converted = std::mbsrtowcs(pDest, &pSrc, buffer.size(), &state);
		if (converted == static_cast<size_t>(-1)) {
			return L"";
		}

		return std::wstring(buffer.data());
	}

	std::string W2N(const std::wstring& wideStr) {
		if (wideStr.empty()) return "";

		std::vector<char> buffer(wideStr.size() * MB_CUR_MAX + 1);
		const wchar_t* pSrc = wideStr.c_str();
		char* pDest = buffer.data();
		std::mbstate_t state = std::mbstate_t();

		size_t converted = std::wcsrtombs(pDest, &pSrc, buffer.size(), &state);
		if (converted == static_cast<size_t>(-1)) {
			return "";
		}

		return std::string(buffer.data());
	}

}

#define VP(pwd) PwdManager::Instance().Verify(::N2W(::GetName()).c_str(), ::N2W(::GetDomain()).c_str(), pwd)

// 全局变量
static bool isSilent{ false };

void ShowHelp() {
	std::cout << FS("wam.general.usage") << "\n\n";
	std::cout << FS("wam.general.options") << "\n";
	std::cout << FS("wam.option.help") << "\n";
	std::cout << FS("wam.option.list") << "\n";
	std::cout << FS("wam.option.version") << "\n";
	std::cout << FS("wam.option.silent") << "\n\n";
	std::cout << FS("wam.general.examples") << "\n";
	std::cout << FS("wam.example.1") << "\n";
	std::cout << FS("wam.example.2") << "\n";
	std::cout << FS("wam.example.3") << "\n";
	std::cout << FS("wam.example.4") << "\n\n";
}

void ShowVersion() {
	std::cout << FS("wam.general.version") << "\n";
	std::cout << FS("wam.general.description") << "\n\n";
}

std::wstring NoEcho() {
	std::wstring password;
	char ch;
	std::cout << FS1("wam.exec.verify", ::GetName());
	while (true) {
		ch = _getwch();
		if (ch == 3) {
			std::cout << "^C" << std::endl;
			std::cout << FS("wam.error.verifyCanceled");
			password.clear();
			exit(1);
		}
		else if (ch == '\r' || ch == '\n') {
			std::cout << std::endl;
			break;
		}
		else if (ch == '\b') {
			if (!password.empty())
			{
				password.pop_back();
			}
		}
		else if (ch >= 32 && ch <= 126) {  // 可打印字符
			password.push_back(ch);
		}
	}
	return password;
}


bool VerifyPassword() {
	if (!VP(NoEcho().c_str())) {
		std::cout << FS("wam.error.retry") << std::endl;
		if (!VP(NoEcho().c_str())) {
			std::cout << FS("wam.error.retry") << std::endl;
			if (!VP(NoEcho().c_str())) {
				return false;
			}
			else {
				return true;
			}
		}
		else {
			return true;
		}
	}
	else {
		return true;
	}
}

// 老版使用临时脚本文件
//bool runElevatedCommand(const std::string& command, const std::string& params = "") {
//    std::string fullCmd = command;
//    if (!params.empty()) {
//        fullCmd += " " + params;
//    }
//
//    // 获取当前工作目录
//    char currentDir[MAX_PATH];
//    GetCurrentDirectoryA(MAX_PATH, currentDir);
//
//    // 构建 PowerShell 脚本文件
//    char tempPath[MAX_PATH];
//    GetTempPathA(MAX_PATH, tempPath);
//
//    std::string psScript = std::string(tempPath) + "wam_exec_" + std::to_string(GetCurrentProcessId()) + ".ps1";
//
//    // 创建 PowerShell 脚本 - 直接在 PowerShell 中执行命令
//    std::ofstream script(psScript);
//    script << "# wam 临时执行脚本\n";
//    script << "# 直接在 PowerShell 中执行命令\n";
//    if (!isSilent) {
//        script << "Write-Host \"" << FS1("wam.exec.cwd", currentDir) << "\" -ForegroundColor Cyan\n";
//        script << "Write-Host \"" << FS1("wam.exec.executing", fullCmd) << "\" -ForegroundColor Cyan\n";
//        script << "Write-Host \"\"\n";
//    }
//    script << "Set-Location -Path \"" << currentDir << "\"\n";
//    script << "Invoke-Expression -Command \"" << fullCmd << "\"\n";
//    script << "Write-Host \"\"\n";
//    script << "Write-Host" << " \"====================^^Cmd:\"" << fullCmd << "\"^^=====================\" -ForegroundColor Gray\n";
//    if (!isSilent) {
//        script << "Read-Host \"" << FS("wam.exec.pause") << "\"\n";
//    }
//    script << "Exit-PSSession";
//    script.close();
//
//
//
//    // 执行 PowerShell 脚本
//    std::string finalParams = "-ExecutionPolicy Bypass -File \"" + psScript + "\"";
//
//    SHELLEXECUTEINFOA shExInfo = { 0 };
//    shExInfo.cbSize = sizeof(shExInfo);
//    shExInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
//    shExInfo.hwnd = 0;
//    shExInfo.lpVerb = "runas";
//    shExInfo.lpFile = "powershell.exe";
//    shExInfo.lpParameters = finalParams.c_str();
//    shExInfo.lpDirectory = 0;
//    shExInfo.nShow = SW_SHOWNORMAL;
//    shExInfo.hInstApp = 0;
//
//    LA(fullCmd);
//    if (!isSilent) {
//        std::cout << FS1("wam.exec.running", fullCmd) << std::endl;
//    }
//
//    if (ShellExecuteExA(&shExInfo)) {
//        WaitForSingleObject(shExInfo.hProcess, INFINITE);
//
//        DWORD exitCode;
//        GetExitCodeProcess(shExInfo.hProcess, &exitCode);
//        CloseHandle(shExInfo.hProcess);
//
//        // 删除临时文件
//        DeleteFileA(psScript.c_str());
//
//        LE(fullCmd, std::to_string(exitCode));
//        if (!isSilent) {
//            std::cout << FS1("wam.exec.completed", std::to_string(exitCode)) << std::endl;
//        }
//        return exitCode == 0;
//    }
//    else {
//        DWORD error = GetLastError();
//        if (error == ERROR_CANCELLED) {
//            LC(fullCmd);
//            std::cerr << FS("wam.error.UACcanceled") << "\n";
//        }
//        else {
//            LF(fullCmd);
//            std::cerr << FS1("wam.error.commandFailed", std::to_string(error)) << "\n";
//        }
//        // 如果失败，清理临时文件
//        DeleteFileA(psScript.c_str());
//        return false;
//    }
//}

bool RunElevatedCommand(const std::string& command, const std::string& params = "") {
	std::string fullCmd = command;
	if (!params.empty()) {
		fullCmd += " " + params;
	}

	char currentDir[MAX_PATH];
	GetCurrentDirectoryA(MAX_PATH, currentDir);

	std::string script;

	// 直接使用命令
	if (!isSilent) {
		script += "Write-Host '" + FS1("wam.exec.cwd", currentDir) + "' -ForegroundColor Cyan; " + "Write-Host '" + FS1("wam.exec.executing", fullCmd) + "' -ForegroundColor Cyan; " + "Write-Host; ";
	}

	script += "Set-Location -Path '" + std::string(currentDir) + "'; " + fullCmd;

	if (!isSilent) {
		script += "; Write-Host '====================^^Cmd:" + fullCmd + "^^=====================' -ForegroundColor Gray; Write-Host; Read-Host '" + FS("wam.exec.pause") + "';";
	}

	// 使用 -Command 参数直接执行
	std::string finalParams = "-ExecutionPolicy Bypass -NoProfile -Command \"" + script + "\"";

	SHELLEXECUTEINFOA shExInfo = { 0 };
	shExInfo.cbSize = sizeof(shExInfo);
	shExInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
	shExInfo.hwnd = 0;
	shExInfo.lpVerb = "runas";
	shExInfo.lpFile = "powershell.exe";
	shExInfo.lpParameters = finalParams.c_str();
	shExInfo.lpDirectory = currentDir;
	shExInfo.nShow = SW_SHOWNORMAL;
	shExInfo.hInstApp = 0;

	LA(fullCmd);
	if (!isSilent) {
		std::cout << FS1("wam.exec.running", fullCmd) << std::endl;
	}

	if (ShellExecuteExA(&shExInfo)) {
		WaitForSingleObject(shExInfo.hProcess, INFINITE);

		DWORD exitCode;
		GetExitCodeProcess(shExInfo.hProcess, &exitCode);
		CloseHandle(shExInfo.hProcess);

		LE(fullCmd, std::to_string(exitCode));
		if (!isSilent) {
			std::cout << FS1("wam.exec.completed", std::to_string(exitCode)) << std::endl;
		}
		return exitCode == 0;
	}
	else {
		DWORD error = GetLastError();
		if (error == ERROR_CANCELLED) {
			LC(fullCmd);
			std::cerr << FS("wam.error.UACcanceled") << "\n";
		}
		else {
			LF(fullCmd);
			std::cerr << FS1("wam.error.commandFailed", std::to_string(error)) << "\n";
		}
		return false;
	}
}

int main(int argc, char* argv[]) {
	// 初始化语言管理器
	LanguageManager::initialize();
	Logger::Initialize();

	if (argc < 2) {
		ShowHelp();
		return 1;
	}

	std::string configPath = ConfigManager::getConfigPath();

	// 解析命令行参数
	for (int i = 1; i < argc; i++) {
		std::string arg = argv[i];

		if (arg == "-h" || arg == "--help") {
			ShowHelp();
			return 0;
		}
		else if (arg == "-v" || arg == "--version") {
			ShowVersion();
			return 0;
		}
		else if (arg == "-l" || arg == "--list") {
			ConfigManager config(configPath);
			std::string username = config.getCurrentUsername();
			auto commands = config.getAllowedCommands(username);

			std::cout << FS1("wam.list.config", configPath) << std::endl;
			std::cout << FS1("wam.list.user", username) << std::endl;
			std::cout << FS("wam.list.allowed") << std::endl;

			if (commands.empty()) {
				std::cout << FS("wam.list.noPermission") << std::endl;
				std::cout << "\n" << FS("wam.list.editHint") << std::endl;
			}
			else {
				for (const auto& cmd : commands) {
					std::cout << "  " << cmd << std::endl;
				}
			}
			return 0;
		}
		else if (arg == "-s" || arg == "--silent") {
			std::cout << FS("wam.exec.silent") << std::endl;
			isSilent = true;
		}
	}

	// 执行命令模式
	if (argc >= 2) {
		std::vector<std::string> args;
		for (int i = 1; i < argc; i++) {
			std::string arg = argv[i];
			if (arg[0] != '-') {
				for (int j = i; j < argc; j++) {
					args.push_back(argv[j]);
				}
				break;
			}
		}

		if (args.empty()) {
			ShowHelp();
			return 1;
		}

		ConfigManager config(configPath);
		std::string username = config.getCurrentUsername();
		std::string command = args[0];

		// 检查权限
		if (!config.isCommandAllowed(username, command)) {
			LD(command);
			std::cerr << FS2("wam.error.noPermission", username, command) << std::endl;
			std::cerr << FS1("wam.error.checkConfig", configPath) << std::endl;

			// 显示配置示例
			std::cout << "\n" << FS("wam.config.example") << std::endl;
			std::cout << FS("wam.config.separator") << std::endl;
			std::cout << FS("wam.config.addToConfig") << std::endl;
			std::cout << FS1("wam.config.userHeader", username) << std::endl;

			// 提取命令基本名
			std::string baseCommand = command;
			size_t spacePos = command.find(' ');
			if (spacePos != std::string::npos) {
				baseCommand = command.substr(0, spacePos);
			}
			size_t lastSlash = baseCommand.find_last_of("\\/");
			if (lastSlash != std::string::npos) {
				baseCommand = baseCommand.substr(lastSlash + 1);
			}

			std::cout << FS1("wam.config.cmdLine", baseCommand) << std::endl;
			std::cout << FS("wam.config.separator") << std::endl;

			return 1;
		}

		// 构建参数
		std::string parameters;
		if (args.size() > 1) {
			for (size_t i = 1; i < args.size(); i++) {
				if (i > 1) parameters += " ";
				parameters += args[i];
			}
		}

		if (!VerifyPassword()) {
			LB(command);
			std::cout << FS("wam.error.verifyFailed") << std::endl;
			return 1;
		}
		else

		// 执行命令
		return RunElevatedCommand(command, parameters) ? 0 : 1;
	}

	ShowHelp();
	return 1;
}