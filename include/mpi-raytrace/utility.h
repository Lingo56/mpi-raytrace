#ifndef UTLITY_H
#define UTLITY_H

#include <cassert>
#include <cmath>
#include <concepts>
#include <cstdlib>
#include <format>
#include <numbers>

#include <quasirand.hpp>
#include <stdexcept>
#include <utility>

// Utility Functions

template <std::floating_point T>
[[nodiscard]] constexpr T degrees_to_radians(T degrees) {
  return degrees * std::numbers::pi / 180.0;
}

template <size_t D, std::floating_point T>
[[nodiscard]]
constexpr auto random_vec(T min = 0.0, T max = 1.0) {
  assert(min <= max);
  __builtin_assume(min <= max);

  static quasirand::QuasiRandom<D> qrng;

  auto res = qrng();
  for (auto &val : res)
    val = std::fma(val, max - min, min);

  return res;
}

// Narrows integers with a runtime check.
template <typename R, typename T>
[[nodiscard]] constexpr R narrow_checked(T value) {
  if (!std::in_range<R>(value))
    throw std::range_error(std::format(
        "Value '{}' of {} cannot fit into {}.",
        value,
        typeid(T).name(),
        typeid(R).name()
    ));

  return static_cast<R>(value);
}

#endif