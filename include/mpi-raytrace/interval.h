#ifndef INTERVAL_H
#define INTERVAL_H

#include <algorithm>
#include <cassert>
#include <concepts>
#include <limits>

constexpr double infinity = std::numeric_limits<double>::infinity();

template <std::totally_ordered T> class Interval {
  T min_, max_;

public:
  constexpr Interval(T min, T max) : min_(min), max_(max) {
    assert(min <= max);
  }

  [[nodiscard]] constexpr T size() const
      noexcept(std::is_nothrow_invocable_r_v<T, decltype(T::operator-), T>) {
    __builtin_assume(min_ <= max_);
    return max_ - min_;
  }

  template <typename U>
    requires std::totally_ordered_with<U, T>
  [[nodiscard]] constexpr bool contains(U x) const noexcept {
    __builtin_assume(min_ <= max_);
    return min_ <= x && x <= max_;
  }

  template <typename U>
    requires std::totally_ordered_with<U, T>
  [[nodiscard]] constexpr bool surrounds(U x) const noexcept {
    __builtin_assume(min_ <= max_);
    return min_ < x && x < max_;
  }

  template <typename U>
    requires std::totally_ordered_with<U, T> && std::convertible_to<T, U>
  [[nodiscard]] constexpr U clamp(U x) const noexcept {
    __builtin_assume(min_ <= max_);
    return std::ranges::clamp(x, min_, max_);
  }

  [[nodiscard]]
  constexpr T min() const noexcept {
    return min_;
  }

  [[nodiscard]]
  constexpr T max() const noexcept {
    return max_;
  }
};

#endif