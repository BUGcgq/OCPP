cmake_minimum_required(VERSION 3.12)
project(YourCProjectName C)

# 设置通用的编译标准等信息
set(CMAKE_C_STANDARD 99)
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR arm)
set(CMAKE_C_COMPILER "/usr/local/arm_linux_4.8/bin/arm-linux-gcc")
set(CMAKE_CXX_COMPILER "/usr/local/arm_linux_4.8/bin/arm-linux-g++")
# 设置你的库名称
set(LIB_NAME "OCPP")

# 添加源文件
aux_source_directory(src SOURCE_FILES)

# 外部库根目录
set(EXT_LIB_DIR "/opt")


# 包含头文件目录
include_directories(
    include
    ${EXT_LIB_DIR}/sqlite-3.33-nuc970/include
    ${EXT_LIB_DIR}/json-c-0.9-nuc970/include
    ${EXT_LIB_DIR}/libwebsocket-nuc970/include
    ${EXT_LIB_DIR}/openssl-1.1.0g-nuc970/include
)


# 需要链接的库
set(COMMON_LINK_LIBS
    -lpthread
    ${EXT_LIB_DIR}/sqlite-3.33-nuc970/lib/libsqlite3.so
    ${EXT_LIB_DIR}/json-c-0.9-nuc970/lib/libjson-c.so
    ${EXT_LIB_DIR}/libwebsocket-nuc970/lib/libwebsockets.so
    ${EXT_LIB_DIR}/openssl-1.1.0g-nuc970/lib/libssl.so
    ${EXT_LIB_DIR}/openssl-1.1.0g-nuc970/lib/libcrypto.so
)

# 创建共享库
add_library(${LIB_NAME}_shared SHARED ${SOURCE_FILES})
set_target_properties(${LIB_NAME}_shared PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}  # 输出路径与编译路径一致
)
target_link_libraries(${LIB_NAME}_shared PUBLIC ${COMMON_LINK_LIBS})

# 创建静态库
add_library(${LIB_NAME}_static STATIC ${SOURCE_FILES})
set_target_properties(${LIB_NAME}_static PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}  # 输出路径与编译路径一致
)
target_link_libraries(${LIB_NAME}_static PUBLIC ${COMMON_LINK_LIBS})
