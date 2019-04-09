
cmake_minimum_required(VERSION 3.1)
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

project(emurpc)

option(EMURPC_BUILD_TESTS "Build tests for emurpc" Off)
option(EMURPC_BUILD_EXAMPLES "Build examples for emurpc" Off)
option(EMURPC_BUILD_STATIC "Build static lib" On)
# TODO support dynamic linking

set(CMAKE_CXX_STANDARD 11)

add_library(emurpc
	include/emurpc.h
	include/emurpc.hpp
    include/common.h
    src/binary_protocol.cpp
	src/binary_protocol.h
    src/emurpc.cpp
    src/json_protocol.cpp
	src/json_protocol.h
	src/protocol.cpp
	src/protocol.h
    src/rpc_client.cpp
	src/rpc_client.h
    src/rpc_server.cpp
	src/rpc_server.h
)

target_include_directories(emurpc PUBLIC include)

# Prevent boost from linking against libs when building
target_compile_definitions(emurpc PRIVATE
	-DBOOST_ERROR_CODE_HEADER_ONLY
    -DBOOST_SYSTEM_NO_LIB
    -DBOOST_DATE_TIME_NO_LIB
	-DBOOST_REGEX_NO_LIB
	-DBOOST_ASIO_DISABLE_BUFFER_DEBUGGING
)
target_include_directories(emurpc PRIVATE externals/boost)
target_include_directories(emurpc PRIVATE externals/json)

if (EMURPC_BUILD_EXAMPLES)
	add_subdirectory(examples)
	# set emurpc as the startup project if the cmake version is high enough
	if(${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.6.0") 
		set(VS_STARTUP_PROJECT emurpc)
	endif()
endif()

if (EMURPC_BUILD_TESTS)
	enable_testing()
	add_subdirectory(test)
	add_test(NAME catch_emurpc COMMAND catch_emurpc)
endif()

