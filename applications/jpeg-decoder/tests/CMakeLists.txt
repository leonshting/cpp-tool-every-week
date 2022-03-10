add_executable(test-jpeg-decoder test-decoder.cpp)
target_link_libraries(test-jpeg-decoder jpeg-decoder GTest::GTest GTest::Main)
target_compile_definitions(test-jpeg-decoder PUBLIC TEST_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/data")

gtest_discover_tests(test-jpeg-decoder)
