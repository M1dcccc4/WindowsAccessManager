#pragma once
#include <string>
#include <vector>
#include <map>
#include <unordered_set>
#include <windows.h>
#include "language.h"

struct UserPermission {
    std::unordered_set<std::string> allowedCommands;
    bool allowAllCommands;

    UserPermission() : allowAllCommands(false) {}
};

class ConfigManager {
private:
    std::map<std::string, UserPermission> userPermissions;
    std::string configPath;
    bool isElevated;

    void parseConfigFile();
    void parseConfigLine(const std::string& line, std::string& currentUser);

public:
    ConfigManager(const std::string& configPath);

    bool isCommandAllowed(const std::string& username,
        const std::string& command);

    void reloadConfig();
    bool isAdminMode() const { return isElevated; }

    std::vector<std::string> getAllowedCommands(
        const std::string& username);

    std::string getCurrentUsername();
    static std::string getConfigPath();
};