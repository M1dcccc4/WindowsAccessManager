# wam - Windows Access Manager

类似 sudo 的 Windows 提权工具，允许受信任的用户以管理员权限执行特定命令。

## 功能特性

- 类似 Unix sudo 的语法和用法
- 基于角色的权限控制
- 支持配置文件管理
- UAC 提权集成
- PowerShell 友好

## 安装

1. 编译程序（未完成）：
   ```bash
   build.bat
2. 将当前目录下build目录放入环境变量PATH中（可选）

## 配置

1. 在Program Data目录下新建wam.ini

2. 编辑wam.ini，添加以下内容：
   ```ini
   [UserName]                                # 你的用户名
   trusted = false                           # 需要密码验证
   allowed = ls, notepad, ipconfig, ...      # 允许的命令

   [TrustedUser]
   truested = true                           # 受信任用户无需密码认证
   allowed = ALL                             # 或者全部允许（受信任的情况下）
## 免责声明
> [!CAUTION]
> 本工具**极其不安全**且**不推荐**在正式工作环境中使用：
> - 非线程安全
> - 潜在的命令注入
> - 编译时需修改头文件
>
> 请谨慎，且只给信任的用户使用。

本工具仅供教育用途。未经你所运行系统的所有者同意，请勿使用。使用本工具，你承认使用本工具导致的任何损害（数据丢失、系统崩溃、黑客入侵）均非作者之责任。