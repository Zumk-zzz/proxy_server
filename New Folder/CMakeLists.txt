cmake_minimum_required(VERSION 3.10)
project(proxy_server)

set(CMAKE_CXX_STANDARD 11)

add_executable(proxy_server main.cpp)

target_link_libraries(proxy_server pthread) # Необходимо для использования epoll