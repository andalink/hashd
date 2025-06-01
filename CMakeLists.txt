cmake_minimum_required(VERSION 3.16)

project(hashd LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(hashd src/main.cpp
    src/server.hpp src/server.cpp
    src/hasher.hpp src/hasher.cpp
    src/socket_descriptor.hpp src/socket_descriptor.cpp
    src/epoll_descriptor.hpp src/epoll_descriptor.cpp
    src/clients_handler.hpp src/clients_handler.cpp)

install(TARGETS hashd DESTINATION ${CMAKE_SOURCE_DIR}/bin)

find_package(OpenSSL REQUIRED)
if(OPENSSL_FOUND)
  include_directories(${OPENSSL_INCLUDE_DIR})
  target_link_libraries(hashd OpenSSL::SSL OpenSSL::Crypto)
endif()
