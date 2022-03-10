#pragma once

#include <cmath>
#include <fftw3.h>

#include "primitives.h"

namespace fft {

struct IDCT88V1 {

    IDCT88V1();
    blocks::Cartesian<double, 8> Transform(const blocks::Cartesian<double, 8> &f);

private:
    const double pi_ = std::atan(1.0) * 4;

    std::array<std::array<double, 8>, 8> cos_xu_;
    std::array<double, 8> cu_;
};

struct IDCT88V2 {

    IDCT88V2();
    ~IDCT88V2();
    blocks::Cartesian<double, 8> Transform(const blocks::Cartesian<double, 8> &f);

private:
    fftw_plan plan_;
    blocks::Block<double, 8> input_, output_;
};

}  // namespace fft
