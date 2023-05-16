cmake_minimum_required(VERSION 3.5)

project(server)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(server server.cpp)

find_package(Threads REQUIRED)
target_link_libraries(server PRIVATE Threads::Threads)
