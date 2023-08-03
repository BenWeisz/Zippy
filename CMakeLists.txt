
cmake_minimum_required(VERSION 3.2 FATAL_ERROR)

project(Zippy-Tests VERSION 1.0)

add_definitions(-std=c++20)
set(CXX_FLAGS "-Wall")
set(CMAKE_CXX_FLAGS, "${CXX_FLAGS}")
set(VENDOR_DIR "~/vendor")

add_executable(Zippy-Tests main.cpp)

find_package(zip)

target_include_directories(Zippy-Tests PUBLIC include)

target_include_directories(Zippy-Tests PUBLIC ${VENDOR_DIR}/libzip-1.10.0/lib)

target_link_libraries(Zippy-Tests zip)
