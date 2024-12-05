#ifndef UTLITY_H
#define UTLITY_H

#include <cassert>
#include <cmath>
#include <concepts>
#include <cstdlib>
#include <limits>
#include <numbers>
#include <random>
#include <stdexcept>
#include <utility>

#include <fmt/format.h>
#include <quasirand.hpp>

#if defined __has_builtin
#if __has_builtin(__builtin_assume)
#define ASSUME(exp) __builtin_assume(exp)
#elif __has_builtin(__builtin_unreachable)
#define ASSUME(exp)                                                            \
  do {                                                                         \
    if (!(exp))                                                                \
      __builtin_unreachable();                                                 \
  } while (false)
#else
#define ASSUME(exp)                                                            \
  do {                                                                         \
  } while (false)
#endif
#endif

// Utility Functions

template <std::floating_point T>
[[nodiscard]] constexpr T degrees_to_radians(T degrees) {
  return degrees * std::numbers::pi / 180.0;
}

// Generates a random `std::array`.
template <size_t D, std::floating_point T>
[[nodiscard]]
auto random_vec(T min = 0.0, T max = 1.0) {
  assert(min <= max);
  ASSUME(min <= max);

  std::random_device rng;
  static std::uniform_real_distribution<double> distribution(0, 1);
  thread_local quasirand::QuasiRandom<D> qrng(distribution(rng));

  auto res = qrng();
  for (auto &val : res)
    val = std::fma(val, max - min, min);

  return res;
}

// `Value` always narrows losslessly into `Target`.
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

// Checked integer narrowing.
template <std::integral R, std::integral T>
  requires(!fits_in<T, R>)
[[nodiscard]] constexpr R try_narrow(T value) {
  if (!std::in_range<R>(value))
    throw std::range_error(fmt::format(
        "Value '{}' of {} cannot fit into {}.",
        value,
        typeid(T).name(),
        typeid(R).name()
    ));

  return static_cast<R>(value);
}

#endif