#pragma once
#include <string>
#include <fstream>

enum class LogType {
	ALLOW,
	ALLOWNPWD,
	BLOCK,
	CANCEL,
	DENY,
	EXIT,
	FAIL,
};

class Logger
{
public:
	static Logger& Instance();
	std::string Log(std::string& command, LogType logType, const std::string& exitCode = "");
	std::string GetTime();
	std::string GetUserAtDomain();
	std::string GetLastAllowed();
	bool AllowNoPassword(std::string timeStr);
	static void Initialize();
private:
	static std::ofstream logFile;
	static std::string filePath;
	Logger() {};
};


#define LA(s) Logger::Instance().Log(s, LogType::ALLOW)
#define LANP(s) Logger::Instance().Log(s, LogType::ALLOWNPWD)
#define LB(s) Logger::Instance().Log(s, LogType::BLOCK)
#define LC(s) Logger::Instance().Log(s, LogType::CANCEL)
#define LD(s) Logger::Instance().Log(s, LogType::DENY)
#define LE(s, i) Logger::Instance().Log(s, LogType::EXIT, i)
#define LF(s) Logger::Instance().Log(s, LogType::FAIL)
