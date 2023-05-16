cmake_minimum_required(VERSION 3.5)

project(client)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(client client.cpp)

find_package(Threads REQUIRED)
target_link_libraries(client PRIVATE Threads::Threads)
