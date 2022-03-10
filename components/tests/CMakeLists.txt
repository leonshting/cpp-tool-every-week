add_executable(test-byte-streams test-byte-streams.cpp)
target_link_libraries(test-byte-streams byte-streams GTest::GTest GTest::Main)

gtest_discover_tests(test-byte-streams)
