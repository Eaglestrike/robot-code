#pragma once

#include <cmath>
#include <cstdlib>

namespace team114 {
namespace c2020 {

template <typename T>
constexpr bool EpsilonEq(T one, T two, T epsilon) noexcept {
    return std::abs(one - two) < epsilon;
}

template <typename T>
constexpr T Deadband(T val, T deadband) noexcept {
    constexpr T kZero = T(0.0);
    if (EpsilonEq(val, kZero, deadband)) {
        return kZero;
    }
    return val;
}

}  // namespace c2020
}  // namespace team114
