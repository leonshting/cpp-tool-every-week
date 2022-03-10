#include "fourier.h"

namespace fft {

IDCT88V1::IDCT88V1() {
    cu_.fill(1);
    cu_[0] = 1.0 / std::sqrt(2);

    for (uint8_t outer = 0; outer < 8; ++outer) {
        for (uint8_t inner = 0; inner < 8; ++inner) {
            double cos_arg = (2 * outer + 1) * pi_ * inner / 16.0;
            cos_xu_[outer][inner] = std::cos(cos_arg);
        }
    }
}

blocks::Cartesian<double, 8> IDCT88V1::Transform(const blocks::Cartesian<double, 8>& f) {
    blocks::Cartesian<double, 8> returned;

    for (auto& r : returned.buffer) {
        r.fill(0.0);
    }

    for (size_t i = 0; i < 8; ++i) {
        for (size_t j = 0; j < 8; ++j) {
            for (size_t outer = 0; outer < 8; ++outer) {
                for (size_t inner = 0; inner < 8; ++inner) {
                    double pre = cu_[outer] * cu_[inner] * f.buffer[outer][inner];
                    double addition = cos_xu_[i][outer] * cos_xu_[j][inner] * pre;
                    returned.buffer[i][j] += 1.0 / 4.0 * addition;
                }
            }
        }
    }
    return returned;
}

IDCT88V2::IDCT88V2() {
    plan_ = fftw_plan_r2r_2d(8, 8, input_.Data(), output_.Data(), FFTW_REDFT01, FFTW_REDFT01,
                             FFTW_ESTIMATE);
}

IDCT88V2::~IDCT88V2() {
    fftw_destroy_plan(plan_);
    fftw_cleanup();
}

blocks::Cartesian<double, 8> IDCT88V2::Transform(const blocks::Cartesian<double, 8>& f) {
    for (uint8_t i = 0; i < 8; ++i) {
        for (uint8_t j = 0; j < 8; ++j) {
            input_.AtIJ(i, j) = f.buffer[i][j] / 8.0;

            if (i > 0) {
                input_.AtIJ(i, j) /= std::sqrt(2.0);
            }

            if (j > 0) {
                input_.AtIJ(i, j) /= std::sqrt(2.0);
            }
        }
    }
    fftw_execute(plan_);
    auto returned = blocks::ToCartesianIJ<double, double, 8>(output_);
    return returned;
}

}  // namespace fft