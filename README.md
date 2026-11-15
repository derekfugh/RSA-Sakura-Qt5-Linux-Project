# RSA-Sakura-Qt (Classic / Cross-Platform)

本仓库提供基于 **Qt 5.15.2** 构建的 RSA-Sakura (Linux) GUI 版本。主要用于兼容旧版操作系统（如 Ubuntu 18.04 及更早版本、Windows 7/8），并提供开箱即用的免安装依赖包。

This repository provides the **Qt 5.15** version of the RSA-Sakura (Linux) GUI. It is designed for compatibility with older operating systems (e.g., Ubuntu 18.04, Windows 7/8) and offers out-of-the-box packages with bundled dependencies.

*(如果你需要最新的 Qt6 版本，请移步 [RSA-Sakura-Qt](https://github.com/derekfugh/RSA-Sakura-Qt))*

## ✨ 特性
*   **无需安装 Qt 环境**：提供包含所有 Qt 依赖库的“完整包”（Full Package），解压即可运行。
*   **自动适配系统**：附带 `install_driver_linux_x64.sh`（Linux）(和 `.bat` 脚本（Windows）)，自动生成桌面快捷方式。

## 🚀 快速开始 (Linux)

1. 前往 [Releases](https://github.com/derekfugh/RSA-Sakura-Qt5-Linux-Project/releases) 页面，下载最新的 `.tar.gz` 或 `.zip` 包。
2. 解压后，在终端中进入该目录。
3. 赋予脚本执行权限并安装：
   ```bash
   chmod +x install.sh
   ./install.sh
（可选）如果你的系统缺少底层 X11 库导致无法启动，请先运行 ./run-debug.sh，它会自动安装基础依赖并显示详细报错信息。

📦 安装与卸载

安装快捷方式：./install_driver_linux_x64.sh

移除快捷方式：./install_driver_linux_x64.sh -u

📦 使用压缩库

解压缩库：当系统未安装Qt5动态库时，解压缩 RSA-Sakura-Qt5-Libs-linux-x64.tar.gz ，将 lib 和 platforms 两个目录，和 RSA-Sakura-Qt 保存在同一目录中。

测试运行：终端里执行 './run-debug.sh' 命令，如 Qt5 系统库全部就绪，RSA-Sakura-Qt 程序将会启动。然后利用 './install_driver_linux_x64.sh' 完成安装。
