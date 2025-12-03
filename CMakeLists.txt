cmake_minimum_required(VERSION 3.21)
project(DatabaseViewer VERSION 1.0.0 LANGUAGES CXX)

# 设置C++标准
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")

# Qt 6 特性配置
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(CMAKE_AUTORCC ON)

# 查找Qt 6组件 (Qt 6中模块命名更规范)
find_package(Qt6 COMPONENTS Core Gui Quick QuickControls2 REQUIRED)
find_package(Qt6 REQUIRED COMPONENTS Core)
find_package(RocksDB CONFIG REQUIRED)
# 资源文件
set(QML_RESOURCES qml.qrc)

# 源文件
set(SOURCES
    main.cpp
)

# 生成可执行文件
qt_add_executable(${PROJECT_NAME}
    ${SOURCES}
    ${QML_RESOURCES}
    icon.rc
    datatransport.h datatransport.cpp
    rocksdbconnect.h rocksdbconnect.cpp
)


if(MSVC)
    # 检查MSVC版本（1930对应VS2022）
    if(MSVC_VERSION GREATER_EQUAL 1930)
        add_compile_definitions(_HAS_TR1_NAMESPACE=0)
        message(STATUS "Defined _HAS_TR1_NAMESPACE=0 for MSVC ${MSVC_VERSION}")
    endif()
endif()

target_link_libraries(DatabaseViewer PRIVATE Qt6::Core)
# Qt 6 模块链接 (使用新的target_link_libraries语法)
target_link_libraries(${PROJECT_NAME}
    PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::Quick
        Qt6::QuickControls2
        RocksDB::rocksdb RocksDB::rocksdb-shared
        Rpcrt4.lib
)

# Qt 6 自动处理QML导入路径
set_target_properties(${PROJECT_NAME} PROPERTIES
    WIN32_EXECUTABLE ON  # Windows下生成GUI程序
    MACOSX_BUNDLE ON     # macOS下生成Bundle
)

# 安装配置
install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin
    BUNDLE DESTINATION .
)
