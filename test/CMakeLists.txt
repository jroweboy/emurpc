
set(TEST_SOURCES
    catch_emurpc.cpp
    json_protocol.cpp
)

add_executable(catch_emurpc ${TEST_SOURCES})
target_include_directories(catch_emurpc PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../src)
target_include_directories(catch_emurpc PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../externals/boost)
target_include_directories(catch_emurpc PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../externals/catch)
target_include_directories(catch_emurpc PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../externals/json)
target_link_libraries(catch_emurpc emurpc)
