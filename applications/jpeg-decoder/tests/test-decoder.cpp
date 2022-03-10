#include <vector>

#include <gtest/gtest.h>

#include "decoder.h"

#ifndef TEST_DATA_DIR
#define TEST_DATA_DIR "."
#endif

std::string ConstructBasePath() {
    std::string result(TEST_DATA_DIR);
    return result;
}

static const std::string kBasePath = ConstructBasePath();

TEST(Decoder, Lenna) {
    auto a = decode::Decode(kBasePath + "/lenna.jpg");
}