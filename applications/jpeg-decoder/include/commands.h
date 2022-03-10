#pragma once

#include <array>
#include <string>
#include <istream>
#include <sstream>
#include <algorithm>

#include "byte-streams.h"
#include "primitives.h"
#include "huffman.h"

namespace commands {

uint16_t GetContentLength(byte_streams::ByteStream &bytes);

struct Start {
    static constexpr std::array<std::uint8_t, 2> kStart = {0xff, 0xd8};
};

struct End {
    static constexpr std::array<std::uint8_t, 2> kStart = {0xff, 0xd9};
};

struct Comment {
    struct Payload {
        std::string comment;
    };

    static constexpr std::array<std::uint8_t, 1> kStart = {0xfe};

    static Payload Read(std::istream &stream);
};

struct DCT {
    struct ChannelProps {
        uint8_t id = 0, horizontal_sp = 0, vertical_sp = 0, dqt_table_id = 0;
    };

    struct Payload {
        uint8_t precision = 8;
        uint16_t height = 0, width = 0;
        std::vector<ChannelProps> channels = {};
    };

    static constexpr std::array<std::uint8_t, 1> kStart = {0xc0};

    static Payload Read(std::istream &stream);
};

struct DQT {
    struct Payload {
        blocks::Block<int16_t, 8> block;
        uint32_t content_length = 0;
        uint8_t precision = 0;
        uint8_t id = 0;
    };

    static constexpr std::array<std::uint8_t, 1> kStart = {0xdb};

    static Payload ReadSingle(std::istream &stream);
    static std::vector<Payload> ReadMultiple(std::istream &stream);

private:
    static Payload ReadStripped(byte_streams::ByteStream &bytes, uint16_t content_length);
};

struct DHT {
    using HuffmanTree = huffman::HuffmanTree<uint8_t, uint8_t>;

    struct Payload {
        HuffmanTree tree;
        uint8_t is_ac = 0, id = 0;
        uint32_t content_length = 0;
    };

    static constexpr std::array<std::uint8_t, 1> kStart = {0xc4};

    static Payload ReadSingle(std::istream &stream);
    static std::vector<Payload> ReadMultiple(std::istream &stream);

private:
    static Payload ReadStripped(byte_streams::ByteStream &bytes);

    static const uint8_t kNumEntries = 16;
};

struct SOS {
    struct ChannelProps {
        uint8_t id, dc_ht_id, ac_ht_id;
    };

    struct Payload {
        std::vector<ChannelProps> channels;
        uint8_t sos, eos, abp;
    };

    static constexpr std::array<std::uint8_t, 1> kStart = {0xda};

    static Payload Read(std::istream &stream);
};

struct App {
    struct Payload {
        std::string exif;
    };

    static constexpr std::array<std::uint8_t, 16> kStart = {0xe0, 0xe1, 0xe2, 0xe3, 0xe4, 0xe5,
                                                            0xe6, 0xe7, 0xe8, 0xe9, 0xea, 0xeb,
                                                            0xec, 0xed, 0xee, 0xef};

    static Payload Read(std::istream &stream);
};

template <class Command>
bool CheckToken(uint8_t token) {
    return std::any_of(Command::kStart.begin(), Command::kStart.end(),
                       [token](auto key) { return key == token; });
}

}  // namespace commands