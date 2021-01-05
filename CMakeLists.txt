cmake_minimum_required(VERSION 3.17 FATAL_ERROR)

project(network-monitor)

# Dependencies
# If dependencies are managed with conan, we set the CMake module path (where
# CMake looks for dependencies) to the current build folder.
if(EXISTS ${CMAKE_CURRENT_BINARY_DIR}/conaninfo.txt)
    set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_BINARY_DIR})
endif()


find_package(Boost 1.75 REQUIRED COMPONENTS system)


set(network-monitor_SOURCES_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(network-monitor_SOURCES "${network-monitor_SOURCES_DIR}/main.cpp")

add_executable(network-monitor ${network-monitor_SOURCES})
target_compile_definitions(network-monitor PRIVATE
    # Boost.Asio
    _WIN32_WINNT=0x0A00
    # Boost.Beast
    BOOST_BEAST_USE_STD_STRING_VIEW=1
)
target_compile_features(network-monitor PRIVATE cxx_std_17)
target_link_libraries(network-monitor PRIVATE Boost::system)
