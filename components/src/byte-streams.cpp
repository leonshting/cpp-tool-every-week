#include "byte-streams.h"

namespace byte_streams {

std::pair<std::uint8_t, std::uint8_t> SplitByte(uint8_t to_split) {
    return {(to_split & 0xf0) >> 4, (to_split & 0x0f)};
}

uint16_t ComposeNBitsBE(uint8_t n, BitStream& stream) {
    uint16_t returned = 0;

    for (uint16_t i = 0; i < n; ++i) {
        auto bit = static_cast<uint8_t>(stream.Yield());
        returned |= (bit << i);
    }

    return returned;
}

uint16_t ComposeNBitsLE(uint8_t n, BitStream& stream) {
    uint16_t returned = 0;

    for (uint8_t i = 0; i < n; ++i) {
        auto bit = static_cast<uint8_t>(stream.Yield());
        returned <<= 1;
        returned += static_cast<uint8_t>(bit);
    }

    return returned;
}

BitStream::BitStream(std::istream& stream, bool staffing) : Stream(stream), staffing_(staffing) {
}

bool BitStream::Yield() {
    if (!current_word_.has_value()) {
        UpdateWord();
    }

    bool read = (current_word_.value() & read_mask_) != 0;
    read_mask_ >>= 1;

    if (read_mask_ == 0) {
        UpdateWord();
    }

    return read;
}

uint8_t BitStream::YieldByte() {
    uint8_t returned = 0;
    for (int8_t bit = 7; bit >= 0; --bit) {
        bool y = Yield();
        returned |= (y << bit);
    }

    return returned;
}

void BitStream::UpdateWord() {
    std::uint8_t read;
    stream_.read(reinterpret_cast<char*>(&read), 1);
    current_word_ = read;
    finished_ = stream_.eof();
    read_mask_ = 1 << 7;

    if (finished_) {
        return;
    }

    if (read == 0xff && staffing_) {
        std::uint8_t staffed;
        stream_.read(reinterpret_cast<char*>(&staffed), 1);

        if (staffed != 0x00 && staffed != 0xff) {
            stream_.seekg(-2, std::ios_base::cur);
            finished_ = true;
            return;
        }
    }
}

ByteStream::ByteStream(std::istream& stream) : Stream(stream) {
}

std::uint8_t ByteStream::Yield() {
    if (finished_) {
        throw std::runtime_error("yielding from closed stream");
    }

    uint8_t read;
    stream_.read(reinterpret_cast<char*>(&read), 1);

    finished_ = stream_.eof();
    return read;
}

}  // namespace byte_streams