cmake_minimum_required(VERSION 3.5)

project(htmlbook LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(htmlbook main.cpp)

add_subdirectory(include)
add_subdirectory(source)
add_subdirectory(3rdparty)
