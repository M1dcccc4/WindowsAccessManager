#include "logger.h"
#include <Windows.h>
#include <secext.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <chrono>
#include <iomanip>
#include <ctime>
#include <vector>

#pragma comment(lib, "Secur32.lib")

std::ofstream Logger::logFile;

// 临时实例
Logger& Logger::Instance() {
	static Logger instant;
	return instant;
}

// 获取时间
std::string Logger::GetTime() {
	std::stringstream timeString;
	auto now = std::chrono::system_clock::now();
	auto now_time_t = std::chrono::system_clock::to_time_t(now);

	std::tm now_tm;
	localtime_s(&now_tm, &now_time_t);

	auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
		now.time_since_epoch()) % 1000;

	timeString << std::put_time(&now_tm, "%Y-%m-%d %H:%M:%S");
	timeString << "." << std::setfill('0') << std::setw(3) << ms.count(); 
	return timeString.str();
}

// 获取用户和域名
std::string Logger::GetUserAtDomain() {                                     // TODO: 为了编译通过请注释 secext.h 里所有的 SEC_ENTRY
	char buffer[256];
	std::string userName, domain, username;									//////////////////////////////////////////////////////////////////////////////////
	std::stringstream fullname;												//  secext.h 里有个怪东西
	DWORD size = sizeof(buffer);											//  
	if (GetUserNameExA(NameSamCompatible, buffer, &size)) {					//  _Success_(return != 0)
		userName = buffer;													//  BOOLEAN
		size_t backslashPos = userName.find('\\');							//  SEC_ENTRY  <------ 我把这里注释了才不报错，其余的函数都是如此，不知道是有意还是无意
																			//  GetUserNameExW(
		if (backslashPos != std::string::npos) {							//		 _In_ EXTENDED_NAME_FORMAT NameFormat,
			domain = userName.substr(0, backslashPos);						//      _Out_writes_to_opt_(*nSize, *nSize) LPWSTR lpNameBuffer,
			username = userName.substr(backslashPos + 1);					//      _Inout_ PULONG nSize
		}																	//  );
		fullname << username << "@" << domain;
		return fullname.str();
	}
	return "UNKNOWN";
}

// 初始化日志管理器
void Logger::Initialize() {
	char* programData = nullptr;
	size_t len = 0;
	std::string logPath;
	if (_dupenv_s(&programData, &len, "PROGRAMDATA") == 0 && programData != nullptr) {
		std::string dirPath = std::string(programData) + "\\wam\\log";
		CreateDirectoryA(dirPath.c_str(), NULL);
		// std::cout << dirPath << std::endl;
		logPath = dirPath + "\\log.txt";
		free(programData);
	}

	if (logPath.empty()) {
		std::cout << "emp" << std::endl;
		logPath = ".\\log.txt";
	}

	Logger::Instance().logFile.open(logPath, std::ios::app);
	if (!Logger::Instance().logFile.is_open()) {
#if defined(ENV_ZH_CN)
		std::cerr << "[wam] 未能打开日志文件" << std::endl;
#elif defined(ENV_EN_US)
		std::cerr << "[wam] Failed to open log file." << std::endl;
#endif // ENV
	}
}

// 将执行结果打入日志
// [xxxx-xx-xx xx:xx:xx] User name@local <verb> 'winver' (code: xx)
std::string Logger::Log(std::string& command, LogType logType, const std::string& exitCode)
{
	std::stringstream fullLog;
	std::vector<std::string> status {
		"<Allowed>",
		"<Blocked>",
		"<Canceled>",
		"<Denied>",
		"<Exited>",
		"<Failed>",
	};

	fullLog << "[" << this->GetTime() << "] " << status[(int)logType] << " user " << this->GetUserAtDomain();
	fullLog << " '" << command << "'";
	if (logType == LogType::EXIT)
		fullLog << " code: " << exitCode;
	fullLog << std::endl;
	logFile << fullLog.str();
	return fullLog.str();
}
