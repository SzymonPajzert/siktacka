cmake_minimum_required(VERSION 3.8)
project(siktacka)

set(CMAKE_CXX_STANDARD 14)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O -Wall -Weffc++ -pedantic  \
    -pedantic-errors -Wextra  -Wcast-align \
    -Wcast-qual  -Wchar-subscripts  -Wcomment \
    -Wdisabled-optimization \
    -Werror -Wfloat-equal  -Wformat  -Wformat=2 \
    -Wformat-nonliteral -Wformat-security  \
    -Wformat-y2k \
    -Wimport  -Winit-self \
    -Winvalid-pch   \
    -Wunsafe-loop-optimizations  -Wlong-long -Wmissing-braces \
    -Wmissing-field-initializers -Wmissing-format-attribute   \
    -Wmissing-include-dirs -Wmissing-noreturn \
    -Wpacked -Wparentheses  -Wpointer-arith \
    -Wredundant-decls -Wreturn-type -Wreorder \
    -Wsequence-point  -Wshadow -Wsign-compare  -Wstack-protector \
    -Wstrict-aliasing -Wstrict-aliasing=2 -Wswitch  -Wswitch-default \
    -Wswitch-enum -Wtrigraphs -Wno-uninitialized \
    -Wunknown-pragmas  -Wunreachable-code -Wunused \
    -Wunused-function  -Wunused-label  -Wunused-parameter \
    -Wunused-value  -Wunused-variable  -Wvariadic-macros \
    -Wvolatile-register-var  -Wwrite-strings")

# TODO -Wuninitialized
# TODO -Wconversion
# TODO add -Wpadded
# TODO add -Winline -Wno-error=inline
# -Waggregate-return - this flag is not useful

include_directories(.)

set(SHARED_SOURCE_FILES
        def/ipaddr.hpp
        def/types.hpp
        parse/parser.hpp def/util.hpp parse/parser.cpp def/types.cpp def/util.cpp)


set(SERVER_SOURCE_FILES
        server/main.cpp
        ${SHARED_SOURCE_FILES} server/GameServer.cpp server/GameServer.hpp server/connect.hpp server/types.hpp server/connect.cpp)

add_executable(siktacka-server ${SERVER_SOURCE_FILES})

set(CLIENT_SOURCE_FILES
        client/main.cpp
        ${SHARED_SOURCE_FILES})
add_executable(siktacka-client ${CLIENT_SOURCE_FILES})

set(UNIT_TEST_FILES
        test/main.cpp)
add_executable(unit-test ${UNIT_TEST_FILES})

set(CLIENT_STUB_FILES
        def/util.cpp
        def/types.cpp
        server/connect.cpp
        parse/parser.cpp
        parse/parser.hpp
        test/client/main.cpp)
add_executable(client-stub ${CLIENT_STUB_FILES})
