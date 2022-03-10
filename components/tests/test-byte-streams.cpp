#include <iostream>
#include <vector>

#include <gtest/gtest.h>

#include "byte-streams.h"

TEST(BitReader, LittleEndiannes) {
    std::stringstream ss;

    ss << char(1) << char(128);
    auto reader = byte_streams::BitStream(ss);

    std::vector<bool> correct_bits = {0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0};
    auto correct_it = correct_bits.begin();
    int count = 0;
    while (!reader.IsFinished()) {
        ASSERT_EQ(reader.Yield(), static_cast<bool>(*correct_it++));
        ++count;
    }
    ASSERT_EQ(count, correct_bits.size());
}

TEST(ComposeBytes, Correcntess) {
    std::stringstream ss;

    ss << char(1) << char(255) << char(255) << char(0) << char(255) << char(0) << char(4);
    auto reader = byte_streams::ByteStream(ss);

    ASSERT_EQ(byte_streams::ComposeBytes<uint16_t>({reader.Yield(), reader.Yield()}), 511);
    uint32_t four_bytes = byte_streams::ComposeBytes<uint32_t>(
        {reader.Yield(), reader.Yield(), reader.Yield(), reader.Yield()});
    ASSERT_EQ(four_bytes, 4278255360);

    uint32_t four_bytes_2 = byte_streams::ComposeBytes<uint32_t>({reader.Yield()});
    ASSERT_EQ(four_bytes_2, 4);
}

TEST(BitReader, MixedReading) {
    std::stringstream ss;

    ss << char(63) << char(127) << char(0);  // 0 0 1 1 1 1 1 1 | 0 1 1 1 1 1 1 1 | 0 ...
    auto reader = byte_streams::BitStream(ss);

    std::vector<uint8_t> correct_words = {0, 126, 1, 1, 248};
    std::vector<uint8_t> actual_words = {reader.Yield(), reader.YieldByte(), reader.Yield(),
                                         reader.Yield(), reader.YieldByte()};

    for (size_t idx = 0; idx < correct_words.size(); ++idx) {
        ASSERT_EQ(correct_words[idx], actual_words[idx]);
    }
}

TEST(BitReader, ReadNBitsLE) {
    std::stringstream ss;

    ss << char(62);  // 0 0 1 1 1 1 1 0
    auto reader = byte_streams::BitStream(ss);

    ASSERT_EQ(byte_streams::ComposeNBitsLE(3, reader), 1);
    ASSERT_EQ(byte_streams::ComposeNBitsLE(3, reader), 7);
    ASSERT_EQ(byte_streams::ComposeNBitsLE(2, reader), 2);
}

TEST(BitReader, ReadNBitsBE) {
    std::stringstream ss;

    ss << char(62);  // 0 0 1 1 1 1 1 0
    auto reader = byte_streams::BitStream(ss);

    ASSERT_EQ(byte_streams::ComposeNBitsBE(3, reader), 4);
    ASSERT_EQ(byte_streams::ComposeNBitsBE(3, reader), 7);
    ASSERT_EQ(byte_streams::ComposeNBitsBE(2, reader), 1);
}
