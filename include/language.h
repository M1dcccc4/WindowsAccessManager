#pragma once
#include <string>
#include <map>
#include <vector>



class LanguageManager {
private:
    std::map<std::string, std::string> messages;
    std::string langPath;
    bool loaded;

    void loadLanguageFile();
    LanguageManager();  // 私有构造函数

public:
    // 删除拷贝构造函数和赋值运算符
    LanguageManager(const LanguageManager&) = delete;
    LanguageManager& operator=(const LanguageManager&) = delete;

    // 获取单例实例
    static LanguageManager& instance();

    // 初始化语言管理器
    static void initialize() {
        instance();  // 确保单例被创建
    }

    // 基础消息获取
    std::string get(const std::string& key, const std::string& defaultValue = "") const;

    // 通用格式化消息获取
    std::string format(const std::string& key, const std::vector<std::string>& args = {},
        const std::string& defaultValue = "") const;

    // 明确的格式化方法
    std::string format1(const std::string& key, const std::string& arg1) const;
    std::string format1(const std::string& key, const std::string& arg1,
        const std::string& defaultValue) const;

    std::string format2(const std::string& key, const std::string& arg1,
        const std::string& arg2) const;
    std::string format2(const std::string& key, const std::string& arg1,
        const std::string& arg2, const std::string& defaultValue) const;

    std::string format3(const std::string& key, const std::string& arg1,
        const std::string& arg2, const std::string& arg3) const;
    std::string format3(const std::string& key, const std::string& arg1,
        const std::string& arg2, const std::string& arg3,
        const std::string& defaultValue) const;

    // 重新加载语言文件
    void reload();

    // 检查是否成功加载
    bool isLoaded() const { return loaded; }

    // 获取语言文件路径
    static std::string getLanguagePath();
};

// 便捷宏
#define FS(key) LanguageManager::instance().get(key)
#define FS1(key, arg1) LanguageManager::instance().format1(key, arg1)
#define FS2(key, arg1, arg2) LanguageManager::instance().format2(key, arg1, arg2)
#define FS3(key, arg1, arg2, arg3) LanguageManager::instance().format3(key, arg1, arg2, arg3)