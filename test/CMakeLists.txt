
set(TEST_SOURCES
    catch_emurpc.cpp
)

add_executable(catch_emurpc ${TEST_SOURCES})
target_include_directories(catch_emurpc PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../src)
target_include_directories(catch_emurpc PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../externals/catch)
target_link_libraries(catch_emurpc emurpc)
