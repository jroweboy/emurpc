
cmake_minimum_required(VERSION 3.0)

project(emurpc)

option(EMURPC_BUILD_TESTS "Build tests for emurpc" Off)
option(EMURPC_BUILD_EXAMPLES "Build examples for emurpc" Off)
option(EMURPC_BUILD_STATIC "Build examples for emurpc" On)
# TODO support dynamic linking
option(EMURPC_BUILD_DYNAMIC "Build examples for emurpc" On)
option(EMURPC_USE_LIBUV "Use libuv for event processing" On)

find_package(Libwebsockets CONFIG REQUIRED)
if (EMURPC_USE_LIBUV)
    # installed with vcpkg
    find_package(unofficial-libuv CONFIG REQUIRED)
endif()

add_library(emurpc
    include/emurpc.h
    include/emurpc/protocol.h
    include/emurpc/types.h
    src/emurpc.c
)

target_include_directories(emurpc PUBLIC include)

target_link_libraries(emurpc PRIVATE websockets_static)
if (EMURPC_USE_LIBUV)
    target_link_libraries(emurpc PRIVATE unofficial::libuv::libuv)
endif()

# set emurpc as the startup project if the cmake version is high enough
if(${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.6.0") 
    set(VS_STARTUP_PROJECT emurpc)
endif()

