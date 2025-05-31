cmake_minimum_required(VERSION 3.16)

project(hashd LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(hashd main.cpp
    server.hpp server.cpp
    hasher.hpp hasher.cpp)

find_package(OpenSSL REQUIRED)
if(OPENSSL_FOUND)
  include_directories(${OPENSSL_INCLUDE_DIR})
  target_link_libraries(hashd OpenSSL::SSL OpenSSL::Crypto)
endif()
