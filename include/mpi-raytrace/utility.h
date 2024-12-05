#ifndef UTLITY_H
#define UTLITY_H

#include <cassert>
#include <cmath>
#include <concepts>
#include <cstdlib>
#include <format>
#include <limits>
#include <numbers>

#include <quasirand.hpp>
#include <random>
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

  std::random_device rng;
  static std::uniform_real_distribution<double> distribution(0, 1);
  thread_local quasirand::QuasiRandom<D> qrng(distribution(rng));

  auto res = qrng();
  for (auto &val : res)
    val = std::fma(val, max - min, min);

  return res;
}

template <typename Value, typename Target>
concept fits_in = requires {
  requires std::integral<Value>;
  requires std::integral<Target>;
  requires std::totally_ordered_with<Value, Target>;

  requires std::cmp_less_equal(
      std::numeric_limits<Target>::max(), std::numeric_limits<Value>::max()
  );
  requires std::cmp_greater_equal(
      std::numeric_limits<Target>::min(), std::numeric_limits<Value>::min()
  );
};

// Narrows integers with a runtime check.
template <std::integral R, std::integral T>
  requires(!fits_in<T, R>)
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