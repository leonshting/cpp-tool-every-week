add_executable(test-byte-streams test-byte-streams.cpp)
add_executable(test-aho-corasick test-aho-corasick.cpp)
add_executable(test-itertools test-itertools.cpp)

target_link_libraries(test-byte-streams byte-streams GTest::GTest GTest::Main)
target_link_libraries(test-aho-corasick aho-corasick GTest::GTest GTest::Main)
target_link_libraries(test-itertools GTest::GTest GTest::Main)

gtest_discover_tests(test-byte-streams)
gtest_discover_tests(test-aho-corasick)
gtest_discover_tests(test-itertools)
