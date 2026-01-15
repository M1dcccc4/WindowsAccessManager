#pragma once
#include <string>
#include <vector>
#include <map>
#include <windows.h>
#include "language.h"

struct UserPermission {
};

class ConfigManager {
private:
    std::string configPath;
    bool isElevated;
    std::vector<std::string> allowedCommands;
    bool allowAllCommands{ false };
    bool noPassword{ false };

public:
    ConfigManager(const std::string& configPath);

    bool isCommandAllowed(const std::string& command);
    bool isNoPassword();

    std::string GetCurrentUsername();
    std::vector<std::string> GetAllowedCommand();
    static std::string GetConfigPath();
    int GetPermission();
};