#pragma once

#include <cmath>
#include <cstdlib>

#include <units/units.h>

namespace team114 {
namespace c2020 {

template <typename T>
constexpr inline bool EpsilonEq(T one, T two, T epsilon) noexcept {
    return std::abs(one - two) < epsilon;
}

template <typename T>
constexpr inline T Deadband(T val, T deadband) noexcept {
    constexpr T kZero = T(0.0);
    if (EpsilonEq(val, kZero, deadband)) {
        return kZero;
    }
    return val;
}

template <typename T>
constexpr inline T Clamp(T val, T min, T max) noexcept {
    if (val < min) {
        return min;
    } else if (val > max) {
        return max;
    } else {
        return val;
    }
}

template <typename T>
constexpr inline units::radian_t DegToRad(T deg) {
    constexpr T conv = M_PI / 180.0;
    return units::radian_t{deg * conv};
}

}  // namespace c2020
}  // namespace team114