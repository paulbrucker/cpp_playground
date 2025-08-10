set(TEST_SOURCES
    test_timeoutQueue.cpp)

add_executable(${PROJECT_NAME}_test ${TEST_SOURCES})
target_link_libraries(${PROJECT_NAME}_test PRIVATE ${PROJECT_NAME}_lib GTest::gtest GTest::gtest_main)
add_test(NAME Tests COMMAND ${PROJECT_NAME}_test)
