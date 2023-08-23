cmake_minimum_required(VERSION 3.12)
project(YourCProjectName C)
set(CMAKE_C_STANDARD 99)
# 设置你的库名，例如 "myevent"
set(LIB_NAME "OCPP")

# 添加源文件路径
aux_source_directory(src SOURCE_FILES)

# 设置头文件路径
include_directories(include)

# 设置交叉编译工具链
set(CMAKE_SYSTEM_NAME Linux)  # 指定操作系统为Linux
set(CMAKE_SYSTEM_PROCESSOR arm)  # 指定处理器架构为arm

# 设置交叉编译工具路径
set (CMAKE_C_COMPILER "/usr/local/arm_linux_4.8/bin/arm-linux-gcc")
set (CMAKE_CXX_COMPILER "/usr/local/arm_linux_4.8/bin/arm-linux-g++")

# 设置编译选项，根据需要自行添加
#set(CMAKE_C_FLAGS "-Wall -O2")
#set(CMAKE_CXX_FLAGS "-Wall -O2")

#添加 sqlite3 动态库
set(sqlite3_libs_DIR /opt/sqlite-3.33-nuc970/lib)                   #添加sqlite3动态库路径
set(sqlite3_headers_DIR /opt/sqlite-3.33-nuc970/include)            #添加sqlite3动态库头路径
include_directories(${sqlite3_headers_DIR})

#添加 json-c 动态库
set(json_c_libs_DIR /opt/json-c-0.9-nuc970/lib)
set(json_c_headers_DIR /opt/json-c-0.9-nuc970/include)
include_directories(${json_c_headers_DIR})

#添加 libwebsocket 动态库
set(libwebsocket_libs_DIR /opt/libwebsocket-nuc970/lib)
set(libwebsocket_headers_DIR /opt/libwebsocket-nuc970/include)
include_directories(${libwebsocket_headers_DIR})

#添加 openSSL 动态库
set(openSSL_libs_DIR /opt/openssl-1.1.0g-nuc970/lib)
set(openSSL_headers_DIR /opt/openssl-1.1.0g-nuc970/include)
include_directories(${openSSL_headers_DIR})

# 编译生成动态库
add_library(${LIB_NAME}_shared SHARED ${SOURCE_FILES})

# 编译生成静态库
add_library(${LIB_NAME}_static STATIC ${SOURCE_FILES})

# 设置动态库输出路径
set_target_properties(${LIB_NAME}_shared PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/lib)

# 设置静态库输出路径
set_target_properties(${LIB_NAME}_static PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/lib)

# 链接pthread库
target_link_libraries(${LIB_NAME}_shared PUBLIC -lpthread)
# 链接SQLite3库
target_link_libraries(${LIB_NAME}_shared PUBLIC ${sqlite3_libs_DIR}/libsqlite3.so)
# 链接json-c库
target_link_libraries(${LIB_NAME}_shared PUBLIC ${json_c_libs_DIR}/libjson-c.so)

# 链接libwebsocket库
target_link_libraries(${LIB_NAME}_shared PUBLIC ${libwebsocket_libs_DIR}/libwebsockets.so)

# 链接OpenSSL库
target_link_libraries(${LIB_NAME}_shared PUBLIC ${openSSL_libs_DIR}/libssl.so)
target_link_libraries(${LIB_NAME}_shared PUBLIC ${openSSL_libs_DIR}/libcrypto.so)

# 链接其他库，根据需要自行添加
# target_link_libraries(${LIB_NAME}_shared PUBLIC other_library)

# 链接动态库时需要添加的路径，根据需要自行添加
target_link_directories(${LIB_NAME}_shared PUBLIC /path/to/other/library)

# 链接静态库时需要添加的路径，根据需要自行添加
target_link_directories(${LIB_NAME}_static PUBLIC /path/to/other/library)

