#pragma once
#include <string>
#include <fstream>

enum class LogType {
	ALLOW,
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
	static void Initialize();
private:
	static std::ofstream logFile;
	Logger() {};
};


#define LA(s) Logger::Instance().Log(s, LogType::ALLOW)
#define LB(s) Logger::Instance().Log(s, LogType::BLOCK)
#define LC(s) Logger::Instance().Log(s, LogType::CANCEL)
#define LD(s) Logger::Instance().Log(s, LogType::DENY)
#define LE(s, i) Logger::Instance().Log(s, LogType::EXIT, i)
#define LF(s) Logger::Instance().Log(s, LogType::FAIL)
