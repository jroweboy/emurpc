
set(TEST_SOURCES
    catch_emurpc.cpp
    json_protocol.cpp
    emurpc.cpp
)

add_executable(catch_emurpc ${TEST_SOURCES})
target_include_directories(catch_emurpc PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../src)
target_include_directories(catch_emurpc PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../externals/boost)
target_include_directories(catch_emurpc PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../externals/catch)
target_include_directories(catch_emurpc PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../externals/json)
target_link_libraries(catch_emurpc emurpc)
target_compile_definitions(catch_emurpc PRIVATE -D_WIN32_WINNT=0x0501 -DWINVER=0x0501)

# Boost is included because we want to connect to the test emulator through websockets
target_compile_definitions(catch_emurpc PRIVATE
	-DBOOST_ERROR_CODE_HEADER_ONLY
    -DBOOST_SYSTEM_NO_LIB
    -DBOOST_DATE_TIME_NO_LIB
	-DBOOST_REGEX_NO_LIB
	-DBOOST_ASIO_DISABLE_BUFFER_DEBUGGING
)
