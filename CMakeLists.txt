cmake_minimum_required(VERSION 3.17 FATAL_ERROR)

project(network-monitor)

# Dependencies
# If dependencies are managed with conan, we set the CMake module path (where
# CMake looks for dependencies) to the current build folder.
if(EXISTS ${CMAKE_CURRENT_BINARY_DIR}/conaninfo.txt)
    set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_BINARY_DIR})
endif()


find_package(Boost 1.75 REQUIRED COMPONENTS system)

# network-monitor LIB
set(network-monitor_SOURCES_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(network-monitor_SOURCES
    ${network-monitor_SOURCES_DIR}/websocket-client.cpp
)
set(network-monitor_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include")

add_library(network-monitor STATIC ${network-monitor_SOURCES})
target_include_directories(network-monitor PUBLIC ${network-monitor_INCLUDE_DIR})
target_link_libraries(network-monitor PUBLIC Boost::system)
target_compile_features(network-monitor PUBLIC cxx_std_17)
target_compile_definitions(network-monitor PUBLIC
    # Boost.Asio
    _WIN32_WINNT=0x0A00
    # Boost.Beast
    BOOST_BEAST_USE_STD_STRING_VIEW=1
)

# network-monitor TESTS
set(network-monitor-tests_SOURCES_DIR "${CMAKE_CURRENT_SOURCE_DIR}/tests")
set(network-monitor-tests_SOURCES
    ${network-monitor-tests_SOURCES_DIR}/test-websocket-client.cpp
)

add_executable(network-monitor-tests ${network-monitor-tests_SOURCES})
target_link_libraries(network-monitor-tests PRIVATE network-monitor)
