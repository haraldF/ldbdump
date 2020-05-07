cmake_minimum_required(VERSION 3.17)
project(ldbdump)

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(external)

add_executable(ldbdump main.cpp)
target_link_libraries(ldbdump PRIVATE leveldb)
