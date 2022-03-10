#include "commands.h"

namespace commands {

uint16_t GetContentLength(byte_streams::ByteStream& bytes) {
    auto length = byte_streams::ComposeBytes<uint16_t>({bytes.Yield(), bytes.Yield()});

    if (length < 2) {
        throw std::runtime_error("block length is less than 2");
    }

    return length - 2;
}

Comment::Payload Comment::Read(std::istream& stream) {
    byte_streams::ByteStream bytes(stream);
    auto content_length = GetContentLength(bytes);

    Payload content;

    std::stringstream content_stream;
    for (uint16_t byte_count = 0; byte_count < content_length; ++byte_count) {
        content_stream << bytes.Yield();
    }
    content.comment = content_stream.str();
    return content;
}

DCT::Payload DCT::Read(std::istream& stream) {
    byte_streams::ByteStream bytes(stream);

    GetContentLength(bytes);

    Payload content;

    content.precision = bytes.Yield();
    content.height = byte_streams::ComposeBytes<uint16_t>({bytes.Yield(), bytes.Yield()});
    content.width = byte_streams::ComposeBytes<uint16_t>({bytes.Yield(), bytes.Yield()});

    content.channels.resize(bytes.Yield());

    for (auto& channel : content.channels) {
        channel.id = bytes.Yield();

        auto sparsity = byte_streams::SplitByte(bytes.Yield());
        channel.horizontal_sp = sparsity.first;
        channel.vertical_sp = sparsity.second;

        channel.dqt_table_id = bytes.Yield();
    }
    return content;
}

DQT::Payload DQT::ReadStripped(byte_streams::ByteStream& bytes, uint16_t content_length) {
    Payload content;

    auto precision_and_id = byte_streams::SplitByte(bytes.Yield());
    content.precision = (precision_and_id.first == 1) ? 2 : 1;
    content.id = precision_and_id.second;

    uint16_t expected_length = 64 * content.precision;

    if (content_length < expected_length + 1) {
        throw std::runtime_error("dqt read failed");
    }

    for (uint16_t byte_count = 0; byte_count < expected_length; ++byte_count) {
        if (content.precision == 1) {
            content.block.At(byte_count) =
                static_cast<int16_t>(byte_streams::ComposeBytes<uint16_t>({bytes.Yield()}));
        } else if (content.precision == 2) {
            content.block.At(byte_count) = static_cast<int16_t>(
                byte_streams::ComposeBytes<uint16_t>({bytes.Yield(), bytes.Yield()}));
        } else {
            throw std::runtime_error("unexpected precision value");
        }
    }

    content.content_length = 1 + expected_length;
    return content;
}

DQT::Payload DQT::ReadSingle(std::istream& stream) {
    byte_streams::ByteStream bytes(stream);
    auto content_length = GetContentLength(bytes);
    return ReadStripped(bytes, content_length);
}

std::vector<DQT::Payload> DQT::ReadMultiple(std::istream& stream) {
    byte_streams::ByteStream bytes(stream);
    auto content_length = GetContentLength(bytes);
    auto content_left = content_length;

    std::vector<Payload> returned;

    while (content_left > 0) {
        returned.push_back(ReadStripped(bytes, content_left));
        content_left -= returned.back().content_length;
    }

    return returned;
}

DHT::Payload DHT::ReadStripped(byte_streams::ByteStream& bytes) {
    Payload content;

    auto meta = byte_streams::SplitByte(bytes.Yield());

    content.is_ac = meta.first;
    content.id = meta.second;

    if (content.is_ac > 1) {
        throw std::runtime_error("invalid ac/dc flag in huffman table");
    }

    uint16_t num_values_accumulated = 0;

    std::vector<uint8_t> values, num_values;

    for (uint8_t entry_id = 1; entry_id < kNumEntries + 1; ++entry_id) {
        num_values.push_back(bytes.Yield());
        num_values_accumulated += num_values.back();
    }

    for (uint16_t value_idx = 0; value_idx < num_values_accumulated; ++value_idx) {
        values.push_back(bytes.Yield());
    }

    content.tree = huffman::HuffmanTree<uint8_t, uint8_t>::FromSequence(num_values, values);
    content.content_length = values.size() + num_values.size() + 1;
    return content;
}

DHT::Payload DHT::ReadSingle(std::istream& stream) {
    byte_streams::ByteStream bytes(stream);
    return ReadStripped(bytes);
}

std::vector<DHT::Payload> DHT::ReadMultiple(std::istream& stream) {
    byte_streams::ByteStream bytes(stream);
    auto content_length = GetContentLength(bytes);
    auto content_left = content_length;

    std::vector<Payload> returned;

    while (content_left > kNumEntries) {
        returned.push_back(ReadStripped(bytes));
        content_left -= returned.back().content_length;
    }

    if (content_left != 0) {
        throw std::runtime_error("huffman tree section not fully consumed");
    }

    return returned;
}

SOS::Payload SOS::Read(std::istream& stream) {
    byte_streams::ByteStream bytes(stream);
    GetContentLength(bytes);

    Payload content;

    auto nr_channels = bytes.Yield();
    content.channels.resize(nr_channels);

    for (auto& channel : content.channels) {
        channel.id = bytes.Yield();
        auto ht_info = byte_streams::SplitByte(bytes.Yield());
        channel.dc_ht_id = ht_info.first;
        channel.ac_ht_id = ht_info.second;
    }

    content.sos = bytes.Yield();
    content.eos = bytes.Yield();
    content.abp = bytes.Yield();

    if (content.sos != 0) {
        throw std::runtime_error("invalid sos parameter");
    }

    if (content.eos != 0x3f) {
        throw std::runtime_error("invalid eos parameter");
    }

    if (content.abp != 0) {
        throw std::runtime_error("invalid abp parameter");
    }

    return content;
}

App::Payload App::Read(std::istream& stream) {
    byte_streams::ByteStream bytes(stream);
    auto content_length = GetContentLength(bytes);

    Payload content;

    std::stringstream content_stream;
    for (uint16_t byte_count = 0; byte_count < content_length; ++byte_count) {
        content_stream << bytes.Yield();
    }
    content.exif = content_stream.str();
    return content;
}

}  // namespace commands
