#include "debug.h"
#include "language.h"
#include <windows.h>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <iostream>
#include <iomanip>
#include <cstdlib>



LanguageManager::LanguageManager() : loaded(false) {
    langPath = getLanguagePath();
    loadLanguageFile();
}

// 获取单例实例
LanguageManager& LanguageManager::instance() {
    static LanguageManager instance;
    return instance;
}

std::string LanguageManager::getLanguagePath() {
    char* programData = nullptr;
    size_t len = 0;

    if (_dupenv_s(&programData, &len, "PROGRAMDATA") == 0 && programData != nullptr) {
        std::string dirPath = std::string(programData) + "\\wam\\lang";
        CreateDirectoryA(dirPath.c_str(), NULL);

#if defined(ENV_ZH_CN)
        std::string langPath = dirPath + "\\zh_CN.cfg";
#elif defined(ENV_EN_US)
        std::string langPath = dirPath + "\\en_US.cfg";
#endif // ENV
        free(programData);
        return langPath;
    }

    if (programData != nullptr) {
        free(programData);
    }

    return "lang\\zh_CN.cfg";
}

void LanguageManager::loadLanguageFile() {
    messages.clear();

    std::ifstream file(langPath);

    if (file.is_open()) {
        std::string line;
        while (std::getline(file, line)) {
            // 跳过注释行和空行
            if (line.empty() || line[0] == '#')
                continue;


            // 查找等号
            size_t eqPos = line.find('=');
            if (eqPos != std::string::npos) {
                std::string key = line.substr(0, eqPos);
                std::string value = line.substr(eqPos + 1);

                // 修剪空白
                key.erase(0, key.find_first_not_of(" \t"));
                key.erase(key.find_last_not_of(" \t") + 1);
                value.erase(0, value.find_first_not_of(" \t"));
                value.erase(value.find_last_not_of(" \t") + 1);

                if (!key.empty() && !value.empty()) {
                    messages[key] = value;  // 覆盖内置配置
                }
            }
        }
        file.close();
        loaded = true;
    } else {
#if defined(ENV_ZH_CN)
        std::cerr << "[wam] 无法打开语言文件" << std::endl;
        std::cerr << "[wam] 检查配置文件: " << langPath << std::endl;
#elif defined(ENV_EN_US)
        std::cerr << "[wam] Failed to open language file." << std::endl;
        std::cerr << "[wam] Check language file: " << langPath << std::endl;
#endif  // ENV
    }
}

std::string LanguageManager::get(const std::string& key, const std::string& defaultValue) const {
    auto it = messages.find(key);
    if (it != messages.end()) {
        return it->second;
    }
    // 如果连内置配置都没有，使用提供的默认值或key本身
    return defaultValue.empty() ? key : defaultValue;
}

std::string LanguageManager::format(const std::string& key, const std::vector<std::string>& args,
    const std::string& defaultValue) const {
    std::string message = get(key, defaultValue);

    // 替换占位符
    for (size_t i = 0; i < args.size(); i++) {
        std::string placeholder = "%s";

        size_t sPos = message.find("%s");

        size_t pos = message.find(placeholder);
        if (pos != std::string::npos) {
            message.replace(pos, placeholder.length(), args[i]);
            pos = message.find(placeholder, pos + args[i].length());
        }
    }

    return message;
}

// 明确的格式化方法
std::string LanguageManager::format1(const std::string& key, const std::string& arg1) const {
    std::vector<std::string> args = { arg1 };
    return format(key, args);
}

std::string LanguageManager::format1(const std::string& key, const std::string& arg1,
    const std::string& defaultValue) const {
    std::vector<std::string> args = { arg1 };
    return format(key, args, defaultValue);
}

std::string LanguageManager::format2(const std::string& key, const std::string& arg1,
    const std::string& arg2) const {
    std::vector<std::string> args = { arg1, arg2 };
    return format(key, args);
}

std::string LanguageManager::format2(const std::string& key, const std::string& arg1,
    const std::string& arg2, const std::string& defaultValue) const {
    std::vector<std::string> args = { arg1, arg2 };
    return format(key, args, defaultValue);
}

std::string LanguageManager::format3(const std::string& key, const std::string& arg1,
    const std::string& arg2, const std::string& arg3) const {
    std::vector<std::string> args = { arg1, arg2, arg3 };
    return format(key, args);
}

std::string LanguageManager::format3(const std::string& key, const std::string& arg1,
    const std::string& arg2, const std::string& arg3,
    const std::string& defaultValue) const {
    std::vector<std::string> args = { arg1, arg2, arg3 };
    return format(key, args, defaultValue);
}

void LanguageManager::reload() {
    loadLanguageFile();
}