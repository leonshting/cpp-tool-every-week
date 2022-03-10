#pragma once

#include <istream>
#include <optional>

namespace byte_streams {

template <class T>
T ComposeBytes(std::initializer_list<uint8_t> bytes) {
    T returned = 0;
    for (auto byte_it = bytes.begin(); byte_it != bytes.end(); ++byte_it) {
        uint16_t offset = 8 * (std::distance(byte_it, bytes.end()) - 1);
        returned |= (*byte_it) << offset;
    }
    return returned;
}

template <class T>
class Stream {
public:
    Stream(std::istream& stream) : stream_(stream), finished_(stream.eof()){};

    virtual T Yield() = 0;

    virtual bool IsFinished() const {
        return finished_;
    }

protected:
    std::istream& stream_;
    bool finished_ = false;
};

class BitStream : public Stream<bool> {
public:
    BitStream(std::istream& stream, bool staffing = true);

    bool Yield() override;

    uint8_t YieldByte();

private:
    void UpdateWord();

    std::optional<std::uint8_t> current_word_ = std::nullopt;
    std::uint8_t read_mask_ = 1 << 7;
    bool staffing_ = true;
};

class ByteStream : public Stream<std::uint8_t> {
public:
    ByteStream(std::istream& stream);

    std::uint8_t Yield() override;
};

uint16_t ComposeNBitsBE(uint8_t n, BitStream& stream);

uint16_t ComposeNBitsLE(uint8_t n, BitStream& stream);

std::pair<std::uint8_t, std::uint8_t> SplitByte(uint8_t to_split);

}  // namespace byte_streams
