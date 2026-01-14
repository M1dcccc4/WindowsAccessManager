#include "config.h"
#include "language.h"
#include <fstream>
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
    parseConfigFile();
}

std::string ConfigManager::getCurrentUsername() {
    char username[UNLEN + 1];
    DWORD username_len = UNLEN + 1;
    if (GetUserNameA(username, &username_len)) {
        return std::string(username);
    }
    return "UNKNOWN";
}

std::string ConfigManager::getConfigPath() {
    // 使用安全的 getenv 替代方法
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

void ConfigManager::parseConfigFile() {
    userPermissions.clear();

    std::ifstream file(configPath);
    if (!file.is_open()) {
        std::cerr << FS1("wam.error.openConfig", configPath) << std::endl;
        return;
    }

    std::string line;
    std::string currentUser;

    while (std::getline(file, line)) {
        parseConfigLine(line, currentUser);
    }
}

void ConfigManager::parseConfigLine(const std::string& line, std::string& currentUser) {
    std::string trimmed = line;

    // 移除注释
    size_t commentPos = trimmed.find('#');
    if (commentPos != std::string::npos) {
        trimmed = trimmed.substr(0, commentPos);
    }

    // 修剪空白
    trimmed.erase(0, trimmed.find_first_not_of(" \t"));
    trimmed.erase(trimmed.find_last_not_of(" \t") + 1);

    if (trimmed.empty()) return;

    if (trimmed[0] == '[' && trimmed.back() == ']') {
        currentUser = trimmed.substr(1, trimmed.length() - 2);
        std::transform(currentUser.begin(), currentUser.end(),
            currentUser.begin(), ::tolower);
        userPermissions[currentUser];
    }
    else if (!currentUser.empty()) {
        std::istringstream iss(trimmed);
        std::string permission;
        iss >> permission;

        if (permission == "ALL") {
            userPermissions[currentUser].allowAllCommands = true;
        }
        else if (permission == "CMD:") {
            std::string command;
            while (iss >> command) {
                userPermissions[currentUser].allowedCommands.insert(command);
            }
        }
    }
}

bool ConfigManager::isCommandAllowed(const std::string& username,
    const std::string& command) {
    std::string userLower = username;
    std::transform(userLower.begin(), userLower.end(),
        userLower.begin(), ::tolower);

    auto it = userPermissions.find(userLower);
    if (it == userPermissions.end()) {
        return false;
    }

    const UserPermission& perm = it->second;
    if (perm.allowAllCommands) {
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
    if (perm.allowedCommands.find(baseCommand) != perm.allowedCommands.end()) {
        return true;
    }

    // 检查不带扩展名的命令名
    if (perm.allowedCommands.find(commandNoExt) != perm.allowedCommands.end()) {
        return true;
    }

    return false;
}

std::vector<std::string> ConfigManager::getAllowedCommands(
    const std::string& username) {
    std::vector<std::string> commands;
    std::string userLower = username;
    std::transform(userLower.begin(), userLower.end(),
        userLower.begin(), ::tolower);

    auto it = userPermissions.find(userLower);
    if (it != userPermissions.end()) {
        const UserPermission& perm = it->second;
        if (perm.allowAllCommands) {
            commands.push_back(FS("wam.list.all"));
        }
        for (const auto& cmd : perm.allowedCommands) {
            commands.push_back(cmd);
        }
    }
    return commands;
}

void ConfigManager::reloadConfig() {
    parseConfigFile();
}

