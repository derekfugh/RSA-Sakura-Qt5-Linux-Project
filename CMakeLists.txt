# 1. 👈 降低最低版本要求，兼容 Ubuntu 18.04 自带的 CMake
cmake_minimum_required(VERSION 3.10)

project(RSA-Sakura-Qt VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 2. 自动探测 Qt6 或 Qt5
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Network)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Network)

# 3. 源文件列表
set(PROJECT_SOURCES
        main.cpp
        driverdialog.cpp driverdialog.h driverdialog.ui
        refreshbutton.cpp refreshbutton.h
        dropoverlay.cpp dropoverlay.h
        keypicker.cpp keypicker.h
        keychainbutton.cpp keychainbutton.h
        resources.qrc
        recentinputfield.cpp recentinputfield.h
        settingshelper.h
        keydatamanager.cpp keydatamanager.h
)

# 4. 👈 统一使用标准 CMake 宏，去掉 Qt6 专属的 qt_add_executable
add_executable(RSA-Sakura-Qt ${PROJECT_SOURCES})

# 👈 包含当前目录，防止找不到头文件
target_include_directories(RSA-Sakura-Qt PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# 5. 链接库
target_link_libraries(RSA-Sakura-Qt PRIVATE 
    Qt${QT_VERSION_MAJOR}::Widgets 
    Qt${QT_VERSION_MAJOR}::Network
)

# 6. 👈 仅在 Qt6 环境下才执行最终化处理
if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(RSA-Sakura-Qt)
endif()

# ==========================================
# 跨平台打包设置
# ==========================================
# Linux 下生成干净的二进制文件
set_target_properties(RSA-Sakura-Qt PROPERTIES
    WIN32_EXECUTABLE TRUE
)

# 安装路径配置
include(GNUInstallDirs)
install(TARGETS RSA-Sakura-Qt
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
