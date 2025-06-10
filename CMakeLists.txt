cmake_minimum_required(VERSION 3.16)

project(hashd LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(ENABLE_TESTS "Enable building tests" ON)
option(TEST_WITH_FETCH_GTEST "Fetch GoogleTest via FetchContent" ON)

set(SOURCE_FILES
    src/server.hpp src/server.cpp
    src/hasher.hpp src/hasher.cpp
    src/socket_descriptor.hpp src/socket_descriptor.cpp
    src/epoll_descriptor.hpp src/epoll_descriptor.cpp
    src/clients_handler.hpp src/clients_handler.cpp)

# hashd server

add_executable(hashd src/main.cpp
    ${SOURCE_FILES})

install(TARGETS hashd DESTINATION ${CMAKE_SOURCE_DIR}/install/bin)

find_package(OpenSSL REQUIRED)
if(OPENSSL_FOUND)
    include_directories(${OPENSSL_INCLUDE_DIR})
    target_link_libraries(hashd OpenSSL::SSL OpenSSL::Crypto)
endif()

# hashd tests

if (ENABLE_TESTS)
    enable_testing()

    if (TEST_WITH_FETCH_GTEST)
        include(FetchContent)
        FetchContent_Declare(googletest
            URL https://github.com/google/googletest/releases/download/v1.17.0/googletest-1.17.0.tar.gz
            DOWNLOAD_EXTRACT_TIMESTAMP 1)
        set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
        FetchContent_MakeAvailable(googletest)
        set(GTEST_TARGETS gtest gtest_main gmock gmock_main)
    else()
        find_package(GTest REQUIRED)
    endif()

    add_executable(hashd_test
        src/tests/server_tests.cpp
        src/tests/hasher_tests.cpp
        src/tests/socket_descriptor_tests.cpp
        ${SOURCE_FILES})
    target_link_libraries(hashd_test OpenSSL::SSL OpenSSL::Crypto gtest gtest_main)

    include(GoogleTest)
    gtest_discover_tests(hashd_test)

    install(
        TARGETS
            hashd_test
            ${GTEST_TARGETS}
        RUNTIME
            DESTINATION ${CMAKE_SOURCE_DIR}/install/bin
        ARCHIVE
            DESTINATION ${CMAKE_SOURCE_DIR}/install/lib)
endif()
