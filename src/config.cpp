#include "config.h"
#include "language.h"
#include <fstream>
#include <vector>
#include <sstream>
#include <algorithm>
#include <lmcons.h>
#include <iostream>
#include <iomanip>
#include <cstdlib>

ConfigManager::ConfigManager(const std::string& configPath)
    : configPath(configPath) {
    isElevated = false;
    HANDLE hToken = NULL;
    if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken)) {
        TOKEN_ELEVATION elevation;
        DWORD dwSize;
        if (GetTokenInformation(hToken, TokenElevation,
            &elevation, sizeof(elevation), &dwSize)) {
            isElevated = elevation.TokenIsElevated != 0;
        }
        CloseHandle(hToken);
    }
    GetPermission();
}

std::string ConfigManager::GetCurrentUsername() {
    char username[UNLEN + 1];
    DWORD username_len = UNLEN + 1;
    if (GetUserNameA(username, &username_len)) {
        return std::string(username);
    }
    return "UNKNOWN";
}

std::vector<std::string> ConfigManager::GetAllowedCommand()
{
    return allowedCommands;
}

std::string ConfigManager::GetConfigPath() {
    // 使用安全的 Getenv 替代方法
    char* programData = nullptr;
    size_t len = 0;

    if (_dupenv_s(&programData, &len, "PROGRAMDATA") == 0 && programData != nullptr) {
        std::string dirPath = std::string(programData) + "\\wam";
        CreateDirectoryA(dirPath.c_str(), NULL);
        std::string configPath = dirPath + "\\wam.ini";
        free(programData);  // 释放内存
        return configPath;
    }

    if (programData != nullptr) {
        free(programData);
    }

    // 回退到当前目录
    char currentDir[MAX_PATH];
    if (GetCurrentDirectoryA(MAX_PATH, currentDir) > 0) {
        return std::string(currentDir) + "\\wam.ini";
    }

    // 如果都失败了，使用默认路径
    return "wam.ini";
}

bool ConfigManager::isCommandAllowed(const std::string& command) {
    if (allowAllCommands) {
        return true;
    }
    std::string baseCommand = command;
    size_t spacePos = command.find(' ');
    if (spacePos != std::string::npos) {
        baseCommand = command.substr(0, spacePos);
    }

    // 检查命令名（不带路径）
    size_t lastSlash = baseCommand.find_last_of("\\/");
    if (lastSlash != std::string::npos) {
        baseCommand = baseCommand.substr(lastSlash + 1);
    }

    // 检查不带扩展名的情况
    std::string commandNoExt = baseCommand;
    size_t dotPos = commandNoExt.find_last_of('.');
    if (dotPos != std::string::npos) {
        commandNoExt = commandNoExt.substr(0, dotPos);
    }

    // 检查完整命令名
    if (std::find(allowedCommands.begin(),
        allowedCommands.end(),
        baseCommand) != allowedCommands.end()) {
        return true;
    }

    // 检查不带扩展名的命令名
    if (std::find(allowedCommands.begin(),
        allowedCommands.end(),
        commandNoExt) != allowedCommands.end()) {
        return true;
    }

    return false;
}

bool ConfigManager::isNoPassword()
{
    return noPassword;
}


// 辅助函数：移除空白字符
void trim(std::string& s) {
    s.erase(0, s.find_first_not_of(" \t"));
    s.erase(s.find_last_not_of(" \t") + 1);
}

int ConfigManager::GetPermission() {
    std::ifstream file(configPath);
    if (!file.is_open()) {
        std::cerr << FS1("wam.error.openConfig", configPath) << std::endl;
        return 1;
    }

    std::string line;
    bool foundUser = false;
    std::string userName = ConfigManager::GetCurrentUsername();
    // 查找用户段
    while (std::getline(file, line)) {
        if (line.find("[" + userName + "]") != std::string::npos) {
            foundUser = true;
            break;
        }
    }

    if (!foundUser) return 1;

    // 读取用户配置直到下一个用户段或文件结束
    while (std::getline(file, line)) {
        // 如果遇到下一个用户段，停止读取
        if (!line.empty() && line[0] == '[') {
            break;
        }

        // 移除注释
        size_t commentPos = line.find('#');
        if (commentPos != std::string::npos) {
            line = line.substr(0, commentPos);
        }

        // 修剪空白
        trim(line);
        if (line.empty()) continue;

        // 解析键值对
        size_t equalPos = line.find('=');
        if (equalPos != std::string::npos) {
            std::string key = line.substr(0, equalPos);
            std::string value = line.substr(equalPos + 1);
            trim(key);
            trim(value);

            if (key == "trusted") {
                noPassword = (value == "true");
            }
            else if (key == "allowed") {
                if (value == "ALL") {
                    allowAllCommands = true;
                }
                else {
                    // 用逗号分割多个命令
                    std::stringstream ss(value);
                    std::string command;
                    while (std::getline(ss, command, ',')) {
                        trim(command);
                        if (!command.empty()) {
                            allowedCommands.push_back(command);
                        }
                    }
                }
            }
        }
    }
    return 0;
}