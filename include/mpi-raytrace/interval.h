#ifndef INTERVAL_H
#define INTERVAL_H

#include <algorithm>
#include <cassert>
#include <concepts>
#include <limits>

#include "utility.h"

constexpr double infinity = std::numeric_limits<double>::infinity();

// Interval [begin(), end()]
template <std::totally_ordered T> class Interval {
  T begin_, end_;

public:
  constexpr Interval(T begin, T end) : begin_(begin), end_(end) {
    assert(begin <= end);
  }

  [[nodiscard]] constexpr T size() const
      noexcept(noexcept(std::declval<T>() - std::declval<T>())) {
    ASSUME(begin_ <= end_);
    return end_ - begin_;
  }

  template <typename U>
    requires std::totally_ordered_with<U, T>
  [[nodiscard]] constexpr bool contains(U x) const noexcept {
    ASSUME(begin_ <= end_);
    return begin_ <= x && x <= end_;
  }

  template <typename U>
    requires std::totally_ordered_with<U, T>
  [[nodiscard]] constexpr bool surrounds(U x) const noexcept {
    ASSUME(begin_ <= end_);
    return begin_ < x && x < end_;
  }

  template <typename U>
    requires std::totally_ordered_with<U, T> && std::convertible_to<T, U>
  [[nodiscard]] constexpr U clamp(U x) const noexcept {
    ASSUME(begin_ <= end_);
    return std::ranges::clamp(x, begin_, end_);
  }

  [[nodiscard]]
  constexpr T begin() const noexcept {
    return begin_;
  }

  [[nodiscard]]
  constexpr T end() const noexcept {
    return end_;
  }
};

#endif