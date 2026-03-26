#include "logger.h"
#include "language.h"
#include <Windows.h>
#include <secext.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <chrono>
#include <iomanip>
#include <ctime>
#include <time.h>
#include <vector>

#pragma comment(lib, "Secur32.lib")

std::ofstream Logger::logFile;
std::string Logger::filePath;

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
		size_t backslashPos = userName.find('\\');							//  SEC_ENTRY  <------ 把这里注释了才不报错，其余的函数都是如此，不知道是有意还是无意
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
	Logger::filePath = logPath;
	Logger::logFile.open(logPath, std::ios::app);
	if (!Logger::Instance().logFile.is_open()) {
		std::cerr << FS("wam.error.openLog") << std::endl;
	}
}

// 将执行结果打入日志
// [xxxx-xx-xx xx:xx:xx] <status> user name@domain 'command' (code: xx)
std::string Logger::Log(std::string& command, LogType logType, const std::string& exitCode)
{
	std::stringstream fullLog;
	std::vector<std::string> status {
		"<Allowed>",
		"<Bypass>",
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

std::string Logger::GetLastAllowed() {
	std::ifstream file(Logger::Instance().filePath, std::ios::ate | std::ios::binary);
	std::string line;
	std::string time{ "1970-01-01 00:00:00" };

	if (!file.is_open()) {
		std::cerr << FS("wam.error.openLog") << std::endl;
		return time;
	}

	// 获取文件大小
	std::streamsize size = file.tellg();
	std::streamsize pos = size;
	int newlinesFound = 0;

	// 从后往前读取
	while (pos > 0) {
		file.seekg(--pos, std::ios::beg);
		char c;
		file.get(c);

		if (c == '\n') {
			if (pos != size - 1) {  // 忽略最后一个字符就是换行的情况
				newlinesFound++;
			}
		}
	}

	// 读取剩余的所有行
	file.seekg(pos + 1, std::ios::beg);
	std::string userAtDomain{ GetUserAtDomain() };
	while (std::getline(file, line)) {
		if ((line.find("user " + userAtDomain) != std::string::npos) &&
			(line.find("<Allowed>") != std::string::npos)) {
			size_t lcolPos = line.find("[");
			size_t rcolPos = line.find("]");
			time = line.substr(lcolPos + 1).substr(0, rcolPos - 1);
		} else continue;
	}

	return time;
}

bool Logger::AllowNoPassword(std::string timeStr)
{
	if (timeStr == "1970-01-01 00:00:00")
		return false;
	std::tm tm = {};
	std::stringstream ss(timeStr);
	std::stringstream sss(GetTime());
	// 按照指定格式解析时间
	ss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");
	// 转换为 time_t
	std::time_t last = std::mktime(&tm);
	sss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");
	std::time_t prev = std::mktime(&tm);
	double time{ std::difftime(prev, last) };
	if (time / 60 <= 5.0) 
		return true;
	else 
		return false;

}
