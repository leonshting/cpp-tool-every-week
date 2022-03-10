#pragma once

#include "rgb-image.h"
#include "commands.h"

#include <string>
#include <vector>
#include <iostream>
#include <fstream>

namespace decode {

struct HuffmanStorage;
struct JPEGMeta;

struct ChannelProps {
    explicit ChannelProps(commands::SOS::ChannelProps init) : props(init), last_dc(0) {
    }
    commands::SOS::ChannelProps props;
    int16_t last_dc = 0;
};

struct Block {
    blocks::Block<int16_t, 8> block;
    uint8_t channel_id;
};

struct CBlock {
    blocks::Cartesian<int16_t, 8> block;
    uint8_t channel_id;
};

struct MCU {

    explicit MCU(uint8_t nr_channels);
    std::vector<std::vector<CBlock>> per_channel_blocks;

    static int16_t GetValue(const std::vector<CBlock>& blocks, int8_t x_offset, int8_t y_offset,
                            const JPEGMeta& meta);

    RGB GetRGB(int8_t x_offset, int8_t y_offset, const JPEGMeta& meta);
};

Image Decode(std::istream& input, const JPEGMeta& meta);

Image Decode(const std::string& filename);

}  // namespace decode
