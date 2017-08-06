cmake_minimum_required(VERSION 3.8)
project(siktacka)

set(CMAKE_CXX_STANDARD 14)

include_directories(.)

set(SERVER_SOURCE_FILES server/main.cpp def/types.hpp parse/parser.hpp)
add_executable(siktacka-server ${SERVER_SOURCE_FILES})

set(CLIENT_SOURCE_FILES client/main.cpp def/types.hpp parse/parser.hpp)
add_executable(siktacka-client ${CLIENT_SOURCE_FILES})