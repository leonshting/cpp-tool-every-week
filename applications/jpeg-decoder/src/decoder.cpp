#include "fourier.h"
#include "decoder.h"

namespace decode {

struct HuffmanStorage {
    std::vector<std::vector<commands::DHT::Payload>> trees;

    static HuffmanStorage FromPayload(const std::vector<commands::DHT::Payload>& parsed_trees);
    const commands::DHT::Payload& Get(uint8_t id, uint8_t is_ac) const;
};

struct JPEGMeta {
    uint16_t width, height, precision;
    uint8_t mcu_x_step = 8, mcu_y_step = 8;
    uint8_t max_granularity_h = 0, max_granularity_v = 0;

    std::vector<commands::Comment::Payload> comments;
    std::vector<commands::App::Payload> app_info;
    std::vector<commands::DQT::Payload> q_tables;
    std::vector<commands::DCT::ChannelProps> channels;

    HuffmanStorage huffman_trees;

    JPEGMeta(std::istream& input);
};

HuffmanStorage HuffmanStorage::FromPayload(
    const std::vector<commands::DHT::Payload>& parsed_trees) {
    HuffmanStorage built;
    uint8_t max_id = 0;
    for (const auto& p : parsed_trees) {
        max_id = std::max(max_id, p.id);
    }

    built.trees.resize(max_id + 1);  // + 1 just for safety
    for (const auto& p : parsed_trees) {
        if (p.tree.root == nullptr) {
            throw std::runtime_error("huffman tree not found");
        }

        if (built.trees[p.id].empty()) {
            built.trees[p.id].resize(2);
        }

        built.trees[p.id][p.is_ac] = p;
    }
    return built;
}

const commands::DHT::Payload& HuffmanStorage::Get(uint8_t id, uint8_t is_ac) const {
    const auto& tree = trees.at(id).at(is_ac);

    if (tree.tree.root == nullptr) {
        throw std::runtime_error("huffman tree not found");
    }

    return tree;
}

JPEGMeta::JPEGMeta(std::istream& input) {
    bool stop;
    auto bytes = byte_streams::ByteStream(input);

    std::vector<commands::DHT::Payload> trees_tmp;
    std::vector<commands::DQT::Payload> q_tables_tmp;
    std::optional<commands::DCT::Payload> dct = std::nullopt;

    uint8_t code = bytes.Yield();

    if (code != 0xff) {
        throw std::runtime_error("jpeg meta read failed: unexpected token");
    }

    if (!commands::CheckToken<commands::Start>(bytes.Yield())) {
        throw std::runtime_error("start token not found");
    }

    do {
        if (bytes.Yield() != 0xff) {
            throw std::runtime_error("jpeg meta read failed: unexpected token");
        }

        code = bytes.Yield();
        stop = commands::CheckToken<commands::SOS>(code);

        if (commands::CheckToken<commands::Comment>(code)) {
            comments.push_back(commands::Comment::Read(input));
        } else if (commands::CheckToken<commands::App>(code)) {
            app_info.push_back(commands::App::Read(input));
        } else if (commands::CheckToken<commands::DQT>(code)) {
            auto payloads = commands::DQT::ReadMultiple(input);
            for (const auto& payload : payloads) {
                q_tables_tmp.push_back(payload);
            }
        } else if (commands::CheckToken<commands::DHT>(code)) {
            auto payloads = commands::DHT::ReadMultiple(input);
            for (const auto& payload : payloads) {
                trees_tmp.push_back(payload);
            }
        } else if (commands::CheckToken<commands::DCT>(code)) {
            if (dct.has_value()) {
                throw std::runtime_error("jpeg meta read failed: duplicated DCT field");
            }
            dct = commands::DCT::Read(input);
        }

    } while (!stop && !bytes.IsFinished());

    if (bytes.IsFinished()) {
        throw std::runtime_error("jpeg meta read failed: unexpected input stop");
    }

    input.seekg(-2, std::ios_base::cur);

    if (!dct.has_value()) {
        throw std::runtime_error("jpeg meta read failed: no dct image info provided");
    }

    if (trees_tmp.empty()) {
        throw std::runtime_error("jpeg meta read failed: no huffman trees provided");
    }

    if (q_tables_tmp.empty()) {
        throw std::runtime_error("jpeg meta read failed: no q tables provided");
    }

    huffman_trees = HuffmanStorage::FromPayload(trees_tmp);

    q_tables.resize(q_tables_tmp.size());
    for (auto& q_table : q_tables_tmp) {
        q_tables[q_table.id] = q_table;
    }

    const auto& dct_value = dct.value();

    width = dct_value.width;
    height = dct_value.height;
    precision = dct_value.precision;

    if (width * height == 0) {
        throw std::runtime_error("empty images not supported");
    }

    channels.resize(dct_value.channels.size() + 1);  // + 1 because of data indexing
    for (auto& props : dct_value.channels) {
        if (props.dqt_table_id >= q_tables.size()) {
            throw std::runtime_error("q table is not defined");
        }

        max_granularity_h = std::max(max_granularity_h, props.horizontal_sp);
        max_granularity_v = std::max(max_granularity_v, props.vertical_sp);
        channels[props.id] = props;
    }

    mcu_x_step = 8 * max_granularity_h;
    mcu_y_step = 8 * max_granularity_v;
}

MCU::MCU(uint8_t nr_channels) : per_channel_blocks(nr_channels) {
}

int16_t MCU::GetValue(const std::vector<CBlock>& blocks, int8_t x_offset, int8_t y_offset,
                      const JPEGMeta& meta) {
    auto channel_id = blocks[0].channel_id;

    const auto props = meta.channels[channel_id];

    if (meta.max_granularity_v == 1 && meta.max_granularity_h == 1) {
        return blocks[0].block.buffer[x_offset][y_offset];
    } else if (meta.max_granularity_v == 2 && meta.max_granularity_h == 1) {
        if (props.vertical_sp == 1) {
            return blocks[0].block.buffer[x_offset][y_offset / 2];
        } else {
            uint8_t index = y_offset > 7 ? 1 : 0;
            return blocks[index].block.buffer[x_offset][y_offset % 8];
        }
    } else if (meta.max_granularity_v == 1 && meta.max_granularity_h == 2) {
        if (props.horizontal_sp == 1) {
            return blocks[0].block.buffer[x_offset / 2][y_offset];
        } else {
            uint8_t index = x_offset > 7 ? 1 : 0;
            return blocks[index].block.buffer[x_offset % 8][y_offset];
        }
    } else {
        if (props.vertical_sp == 1 && props.horizontal_sp == 1) {
            return blocks[0].block.buffer[x_offset / 2][y_offset / 2];
        } else if (props.vertical_sp == 2 && props.horizontal_sp == 1) {
            uint8_t x_index = x_offset > 7 ? 1 : 0;
            return blocks[x_index].block.buffer[x_offset % 8][y_offset / 8];
        } else if (props.vertical_sp == 1 && props.horizontal_sp == 2) {
            uint8_t y_index = y_offset > 7 ? 1 : 0;
            return blocks[y_index].block.buffer[x_offset / 2][y_offset % 8];
        } else {
            uint8_t y_index = y_offset > 7 ? 1 : 0;
            uint8_t x_index = x_offset > 7 ? 1 : 0;

            uint8_t index = 2 * y_index + x_index;
            return blocks[index].block.buffer[x_offset % 8][y_offset % 8];
        }
    }
}

RGB MCU::GetRGB(int8_t x_offset, int8_t y_offset, const JPEGMeta& meta) {
    int16_t y, cb = 128, cr = 128;

    y = GetValue(per_channel_blocks[0], x_offset, y_offset, meta);
    if (per_channel_blocks.size() == 3) {
        cb = GetValue(per_channel_blocks[1], x_offset, y_offset, meta);
        cr = GetValue(per_channel_blocks[2], x_offset, y_offset, meta);
    }

    int r = std::round(y + 1.402 * (cr - 128));
    int g = std::round(y - 0.34414 * (cb - 128) - 0.71414 * (cr - 128));
    int b = std::round(y + 1.772 * (cb - 128));

    return {
        std::max(0, std::min(255, r)),
        std::max(0, std::min(255, g)),
        std::max(0, std::min(255, b)),
    };
}

uint8_t DecodeByte(byte_streams::BitStream& bits, const commands::DHT::HuffmanTree& tree) {
    auto current = tree.root.get();

    while (!current->IsTerminal()) {
        bool bit = bits.Yield();

        current = bit ? current->right : current->left;

        if (current == nullptr) {
            throw std::runtime_error("error getting huffman code");
        }
    }
    return current->value.value();
}

int16_t MaybeNegate(uint8_t num_bits, int16_t raw) {
    if ((raw >> (num_bits - 1)) == 0) {
        return raw - (1 << num_bits) + 1;
    }
    return raw;
}

CBlock DecodeBlock(byte_streams::BitStream& bits, ChannelProps& channel, fft::IDCT88V2& idct,
                   const JPEGMeta& meta) {
    Block decoded;
    decoded.channel_id = channel.props.id;
    decoded.block.buffer.fill(0);

    const auto& dc_tree = meta.huffman_trees.Get(channel.props.dc_ht_id, 0);
    const auto& ac_tree = meta.huffman_trees.Get(channel.props.ac_ht_id, 1);

    auto dc_byte = DecodeByte(bits, dc_tree.tree);

    auto decoded_it = decoded.block.buffer.begin();

    if (dc_byte == 0x00) {
        *decoded_it = dc_byte;
    } else {
        auto [num_zeros, num_bits] = byte_streams::SplitByte(dc_byte);
        auto abs_c = byte_streams::ComposeNBitsLE(num_bits, bits);
        auto negated = MaybeNegate(num_bits, abs_c);
        *decoded_it = negated;
    }

    decoded.block.buffer[0] += channel.last_dc;
    channel.last_dc = decoded.block.buffer[0];

    decoded_it = std::next(decoded_it);

    while (decoded_it != decoded.block.buffer.end() && !bits.IsFinished()) {
        auto ac_byte = DecodeByte(bits, ac_tree.tree);
        if (ac_byte == 0x00) {
            break;
        } else if (ac_byte == 0xf0) {
            decoded_it = std::next(decoded_it, 16);
        } else {
            auto [num_zeros, num_bits] = byte_streams::SplitByte(ac_byte);
            decoded_it = std::next(decoded_it, num_zeros);

            auto abs_c = byte_streams::ComposeNBitsLE(num_bits, bits);
            *decoded_it = MaybeNegate(num_bits, abs_c);
            decoded_it = std::next(decoded_it);
        }
    }

    const auto& rescaler = meta.q_tables[meta.channels[channel.props.id].dqt_table_id].block;
    decoded.block = decoded.block * rescaler;

    auto cartesian = blocks::ToCartesianZZ<int16_t, double, 8>(decoded.block);
    auto fft = idct.Transform(cartesian);
    auto fft16 = blocks::As<double, std::int16_t, 8>(fft);

    auto clipped = Clip(fft16 + std::int16_t(128), std::int16_t(0), std::int16_t(255));
    return {clipped, decoded.channel_id};
}

MCU DecodeMCU(byte_streams::BitStream& bits, std::vector<ChannelProps>& props, fft::IDCT88V2& idct,
              const JPEGMeta& meta) {

    MCU decoded(props.size());

    for (uint8_t channel_idx = 0; channel_idx < props.size(); ++channel_idx) {
        auto& channel = props[channel_idx];
        auto& card = meta.channels[channel.props.id];

        uint8_t num_blocks = card.horizontal_sp * card.vertical_sp;

        for (uint8_t block_idx = 0; block_idx < num_blocks; ++block_idx) {
            auto block = DecodeBlock(bits, channel, idct, meta);
            decoded.per_channel_blocks[channel_idx].push_back(block);
        }
    }

    return decoded;
}

Image Decode(std::istream& input, const JPEGMeta& meta) {
    auto bytes = byte_streams::ByteStream(input);
    if (bytes.Yield() != 0xff) {
        throw std::runtime_error("0xff expected");
    }

    if (bytes.Yield() != 0xda) {
        throw std::runtime_error("0xda expected");
    }

    auto idct = fft::IDCT88V2();

    auto sos = commands::SOS::Read(input);
    auto bits = byte_streams::BitStream(input, true);

    std::vector<ChannelProps> props;

    for (const auto& sos_prop : sos.channels) {
        props.emplace_back(sos_prop);

        if (props.back().props.id >= meta.channels.size()) {
            throw std::runtime_error("unexpected channel id in sos information");
        }
    }

    Image image(meta.width, meta.height);
    uint16_t cur_x = 0, cur_y = 0;

    while (!bits.IsFinished() && cur_x < meta.width && cur_y < meta.height) {
        auto mcu = DecodeMCU(bits, props, idct, meta);
        for (uint8_t x_offset = 0; x_offset < meta.mcu_x_step; ++x_offset) {
            for (uint8_t y_offset = 0; y_offset < meta.mcu_y_step; ++y_offset) {
                uint16_t x_image = x_offset + cur_x, y_image = y_offset + cur_y;

                if (x_image >= meta.width || y_image >= meta.height) {
                    continue;
                }

                auto rgb = mcu.GetRGB(x_offset, y_offset, meta);
                image.SetPixel(y_image, x_image, rgb);
            }
        }
        cur_x += meta.mcu_x_step;
        if (cur_x >= meta.width) {
            cur_x = 0;
            cur_y += meta.mcu_y_step;
        }
    }

    if (bytes.Yield() != 0xff) {
        throw std::runtime_error("0xff expected: premature end of image");
    }

    if (bytes.Yield() != 0xd9) {
        throw std::runtime_error("0xd9 expected: premature end of image");
    }

    return image;
}

Image Decode(const std::string& filename) {
    std::ifstream jpeg_stream(filename, std::ios::binary);

    decode::JPEGMeta meta(jpeg_stream);

    auto image = decode::Decode(jpeg_stream, meta);

    if (!meta.comments.empty()) {
        image.SetComment(meta.comments.back().comment);
    }

    return image;
}

}  // namespace decode
