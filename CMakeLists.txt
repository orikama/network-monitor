cmake_minimum_required(VERSION 3.17 FATAL_ERROR)

project(network-monitor)

set(network-monitor_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp")

add_executable(network-monitor ${network-monitor_SOURCES})
target_compile_features(network-monitor PRIVATE cxx_std_11)
