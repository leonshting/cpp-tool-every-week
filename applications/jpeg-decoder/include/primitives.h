#pragma once

#include <array>

namespace blocks {

template <class T, uint8_t Size = 8>
struct Cartesian {
    std::array<std::array<T, Size>, Size> buffer;
};

template <class T, uint8_t Size = 8>
struct Block {
    T AtZZ(uint8_t row, uint8_t col) const {
        uint8_t offset;
        uint16_t diag_id;

        if (row + col < Size) {
            diag_id = row + col;
            offset = (diag_id + 1) * diag_id / 2;
            return buffer[offset + ((diag_id % 2) ? col : row)];
        } else {
            col = Size - 1 - col;
            row = Size - 1 - row;
            diag_id = row + col;
            offset = (diag_id + 1) * diag_id / 2;
            uint16_t index = offset + ((diag_id % 2) ? col : row);
            return buffer[Size * Size - 1 - index];
        }
    }

    T AtIJ(uint8_t row, uint8_t col) const {
        return buffer[row * Size + col];
    }

    T &AtIJ(uint8_t row, uint8_t col) {
        return buffer[row * Size + col];
    }

    T At(uint8_t buffer_pos) const {
        return buffer[buffer_pos];
    }

    T &At(uint8_t buffer_pos) {
        return buffer[buffer_pos];
    }

    T *Data() {
        return buffer.data();
    }

    std::array<T, Size * Size> buffer;
};

template <typename From, typename To, uint8_t Size>
Cartesian<To, Size> ToCartesianZZ(const Block<From, Size> &from) {
    Cartesian<To, Size> cartesian;

    for (size_t i = 0; i < Size; ++i) {
        for (size_t j = 0; j < Size; ++j) {
            cartesian.buffer[i][j] = static_cast<To>(from.AtZZ(i, j));
        }
    }
    return cartesian;
}

template <typename From, typename To, uint8_t Size>
Cartesian<To, Size> ToCartesianIJ(const Block<From, Size> &from) {
    Cartesian<To, Size> cartesian;

    for (size_t i = 0; i < Size; ++i) {
        for (size_t j = 0; j < Size; ++j) {
            cartesian.buffer[i][j] = static_cast<To>(from.AtIJ(i, j));
        }
    }
    return cartesian;
}

template <typename T, uint8_t Size>
Cartesian<T, Size> Clip(const Cartesian<T, Size> &from, T min, T max) {
    Cartesian<T, Size> cartesian;

    for (size_t i = 0; i < Size; ++i) {
        for (size_t j = 0; j < Size; ++j) {
            cartesian.buffer[i][j] = std::max(min, std::min(max, (from.buffer[i][j])));
        }
    }
    return cartesian;
}

template <typename From, typename To, uint8_t Size>
Block<To, Size> As(const Block<From, Size> &from) {
    Block<To, Size> result;
    for (uint8_t count = 0; count < result.buffer.size(); ++count) {
        result.buffer[count] = static_cast<To>(from.buffer[count]);
    }
    return result;
}

template <typename From, typename To, uint8_t Size>
Cartesian<To, Size> As(const Cartesian<From, Size> &from) {
    Cartesian<To, Size> cartesian;

    for (size_t i = 0; i < Size; ++i) {
        for (size_t j = 0; j < Size; ++j) {
            cartesian.buffer[i][j] = static_cast<To>(from.buffer[i][j]);
        }
    }
    return cartesian;
}

}  // namespace blocks

template <typename T, uint8_t Size>
blocks::Block<T, Size> operator+(const blocks::Block<T, Size> &lhs,
                                 const blocks::Block<T, Size> &rhs) {

    auto result = lhs;
    for (uint8_t count = 0; count < result.buffer.size(); ++count) {
        result.buffer[count] += rhs.buffer[count];
    }
    return result;
}

template <typename T, uint8_t Size>
blocks::Block<T, Size> operator*(const blocks::Block<T, Size> &lhs,
                                 const blocks::Block<T, Size> &rhs) {
    auto result = lhs;
    for (uint8_t count = 0; count < result.buffer.size(); ++count) {
        result.buffer[count] *= rhs.buffer[count];
    }
    return result;
}

template <typename T, uint8_t Size>
blocks::Cartesian<T, Size> operator+(const blocks::Cartesian<T, Size> &from, T scalar) {
    blocks::Cartesian<T, Size> cartesian;

    for (size_t i = 0; i < Size; ++i) {
        for (size_t j = 0; j < Size; ++j) {
            cartesian.buffer[i][j] = from.buffer[i][j] + scalar;
        }
    }
    return cartesian;
}

template <typename T, uint8_t Size>
blocks::Cartesian<T, Size> operator-(const blocks::Cartesian<T, Size> &l,
                                     const blocks::Cartesian<T, Size> &r) {
    blocks::Cartesian<T, Size> cartesian;

    for (size_t i = 0; i < Size; ++i) {
        for (size_t j = 0; j < Size; ++j) {
            cartesian.buffer[i][j] = l.buffer[i][j] - r.buffer[i][j];
        }
    }
    return cartesian;
}
