
cmake_minimum_required(VERSION 3.0)

project(emurpc)

option(EMURPC_BUILD_TESTS "Build tests for emurpc" Off)
option(EMURPC_BUILD_EXAMPLES "Build examples for emurpc" On)
option(EMURPC_BUILD_STATIC "Build static lib" On)
# TODO support dynamic linking
option(EMURPC_USE_THREAD "Use a separate thread for event processing" On)

add_library(emurpc
    include/emurpc.h
    include/emurpc/protocol.h
    include/emurpc/types.h
    src/emurpc.c
    src/rpc_client.c
	src/rpc_client.h
    src/rpc_server.c
	src/rpc_server.h
    src/binary_protocol.c
	src/binary_protocol.h
    src/json_protocol.c
	src/json_protocol.h
)

target_include_directories(emurpc PUBLIC include)

find_package(Libwebsockets NO_DEFAULT_PATH)
if (NOT Libwebsockets_FOUND)
	message("Falling back to compiling libwebsocket from externals")
	# pass in our compile options to libwebsockets
	option(LWS_WITHOUT_CLIENT "" On)
	option(LWS_WITHOUT_TESTAPPS "" On)
	option(LWS_WITHOUT_TEST_PING "" On)
	option(LWS_WITHOUT_TEST_CLIENT "" On)
	option(LWS_WITHOUT_TEST_SERVER "" On)
	option(LWS_WITHOUT_TEST_SERVER_EXTPOLL "" On)
	option(LWS_WITH_SSL "" Off)
	option(LWS_WITH_HTTP2 "" Off)
	if (EMURPC_BUILD_STATIC)
		option(LWS_WITH_SHARED "" Off)
		option(LWS_WITH_STATIC "" On)
	else()
		option(LWS_WITH_SHARED "" On)
		option(LWS_WITH_STATIC "" Off)
	endif()
	add_subdirectory(externals/libwebsockets)
	message(${LIBWEBSOCKETS_INCLUDE_DIRS})
	target_include_directories(emurpc PRIVATE externals/libwebsockets/include)
	target_include_directories(emurpc PRIVATE ${LIBWEBSOCKETS_INCLUDE_DIRS})
else()
	message("Using system libwebsockets")
	find_path(LIBWEBSOCKETS_INCLUDE_DIR NAMES libwebsockets.h)
	target_include_directories(emurpc PRIVATE ${LIBWEBSOCKETS_INCLUDE_DIR})
endif()

if (EMURPC_BUILD_STATIC AND MSVC)
	# override vcpkg target triplet to point to the static library
	if (VCPKG_TARGET_TRIPLET)
		if(CMAKE_SIZEOF_VOID_P EQUAL 8)
			set(VCPKG_TARGET_TRIPLET "x64-windows-static" CACHE STRING "" FORCE)
		else()
			set(VCPKG_TARGET_TRIPLET "x86-windows-static" CACHE STRING "" FORCE)
		endif()
	endif()
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
endif()

target_link_libraries(emurpc PUBLIC websockets)

if (EMURPC_USE_THREAD)
	add_library(tinycthread
		externals/tinycthread/tinycthread.h
		externals/tinycthread/tinycthread.c
	)
	target_link_libraries(emurpc PRIVATE tinycthread)
	target_include_directories(emurpc PRIVATE externals/tinycthread/)
	target_compile_definitions(emurpc PRIVATE HAVE_THREADS=1)
endif()

if (EMURPC_BUILD_EXAMPLES)
	add_subdirectory(examples)
	# set emurpc as the startup project if the cmake version is high enough
	if(${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.6.0") 
		set(VS_STARTUP_PROJECT emurpc)
	endif()
endif()
