# wam - Windows Access Manager

类似 sudo 的 Windows 提权工具，允许受信任的用户以管理员权限执行特定命令。

## 介绍

使用 WindowsAPI 中 `ShellExecuteEx` 创建独立 shell 进程，执行提权的 powershell 脚本，有以下选项可选：
```shell
-h --help       显示帮助
-v --version    显示版本
-s --silent     静默执行
```

## 功能特性

- 类似 sudo 的语法和用法
- 基于角色的权限控制
- 支持配置文件管理
- UAC 提权集成
- PowerShell 友好

## 编译

如果您想从源码编译，请将 `secext.h` 里所有的 `SEC_ENTRY` 注释，示例如下：
   ```cpp
   _Success_(return != 0)
   BOOLEAN
   // SEC_ENTRY        <------ 把这里注释了才不报错
   GetUserNameExW(
      _In_ EXTENDED_NAME_FORMAT NameFormat,
      _Out_writes_to_opt_(*nSize, *nSize) LPWSTR lpNameBuffer,
      _Inout_ PULONG nSize
   );
   ```
编译时请在预定义宏中加上 `ENV_ZH_CN` 或 `ENV_EN_US` 以在非常情况下获得中文/英文提示
将编译产生的.exe文件放入环境变量中

## 安装

1. 将 release 压缩包解压
2. 将 `\bin` 中可执行文件放入环境变量中
3. 将 `\language` 中的.cfg文件放入 `\ProgramData\wam\lang` 目录下

## 配置

1. 在 `\ProgramData\wam` 目录下创建 `wam.ini`

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
> - 管道操作不友好
> - 无法最小化权限
>
> 请谨慎，且只给信任的用户使用。

本工具仅供教育用途。未经你所运行系统的所有者同意，请勿使用。使用本工具，你承认使用本工具导致的任何损害（数据丢失、系统崩溃、黑客入侵）均非作者之责任。