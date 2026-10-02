cmake_minimum_required(VERSION 3.16)
project(msgroom)

include_directories(3rd)


# 服务端可执行文件：msg_server.exe，只编译server.cpp
add_executable(msg_server server.cpp)
target_link_libraries(msg_server PRIVATE ws2_32)
target_precompile_headers(msg_server PRIVATE pch.h)

# 客户端可执行文件：msg_client.exe，只编译client.cpp
add_executable(msg_client client.cpp)
target_link_libraries(msg_client PRIVATE ws2_32)
target_precompile_headers(msg_client PRIVATE pch.h)
