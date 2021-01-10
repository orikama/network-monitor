cmake_minimum_required(VERSION 3.17 FATAL_ERROR)

project(network-monitor CXX)

# Dependencies
# If dependencies are managed with conan, we set the CMake module path (where
# CMake looks for dependencies) to the current build folder.
if(EXISTS ${CMAKE_CURRENT_BINARY_DIR}/conaninfo.txt)
    set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_BINARY_DIR})
endif()


find_package(Boost 1.75 REQUIRED COMPONENTS system unit_test_framework)
find_package(OpenSSL 1.1.1 REQUIRED)


# ----------- network-monitor LIB -----------
set(network-monitor_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(network-monitor_SRC
    ${network-monitor_SRC_DIR}/websocket-client.cpp
)

set(network-monitor_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include")
set(network-monitor_HEADERS_DIR "${network-monitor_INCLUDE_DIR}/network-monitor")
set(network-monitor_HEADERS
    ${network-monitor_HEADERS_DIR}/websocket-client.hpp
)

add_library(network-monitor STATIC ${network-monitor_SRC} ${network-monitor_HEADERS})
target_include_directories(network-monitor PUBLIC ${network-monitor_INCLUDE_DIR})
target_link_libraries(network-monitor
    PUBLIC Boost::system
           OpenSSL::OpenSSL
)
target_compile_features(network-monitor PUBLIC cxx_std_17)
target_compile_definitions(network-monitor PUBLIC
    # Boost.Asio
    _WIN32_WINNT=0x0A00
    # Boost.Beast
    BOOST_BEAST_USE_STD_STRING_VIEW=1
)


# ----------- network-monitor TESTS -----------
# NOTE: I feel like this is still wrong, at least I should move this to tests/CMakeLists.txt
# https://cliutils.gitlab.io/modern-cmake/chapters/testing.html
if(${CMAKE_PROJECT_NAME} STREQUAL ${PROJECT_NAME})
    include(CTest)

    #if(${BUILD_TESTING})
        #add_subdirectory(tests)
    set(network-monitor-tests_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/tests")
    set(network-monitor-tests_SRC
        ${network-monitor-tests_SRC_DIR}/main.cpp
        ${network-monitor-tests_SRC_DIR}/test-websocket-client.cpp
    )
    
    add_executable(network-monitor-tests ${network-monitor-tests_SRC})
    target_link_libraries(network-monitor-tests
        PRIVATE network-monitor
                Boost::unit_test_framework
    )
    target_compile_definitions(network-monitor-tests
        PRIVATE TESTS_CACERT_PEM="${network-monitor-tests_SRC_DIR}/cacert.pem"
    )
    
    add_test(
        NAME network-monitor-tests
        COMMAND $<TARGET_FILE:network-monitor-tests>
    )
    #endif()
endif()
