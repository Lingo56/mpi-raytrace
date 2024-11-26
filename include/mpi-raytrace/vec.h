#ifndef VEC_H
#define VEC_H

#include "blaze/math/TransposeFlag.h"
#include "blaze/math/expressions/Vector.h"
#include <cmath>

#include <blaze/Blaze.h>
#include <blaze/math/dense/StaticVector.h>
#include <concepts>
#include <initializer_list>

struct Vec3 : public blaze::StaticVector<double, 3UL> {
  using Base = blaze::StaticVector<double, 3UL>;

  constexpr Vec3() noexcept : Base{0.0, 0.0, 0.0} {}

  template <typename U>
    requires std::constructible_from<Base, U>
  constexpr explicit Vec3(U &&arg) noexcept : Base(std::forward<U>(arg)) {}

  // template <typename... Args>
  // explicit constexpr Vec3(Args &&...args) noexcept
  //     : Base({std::forward<Args>(args)...}) {}

  constexpr Vec3(std::initializer_list<double> args) noexcept : Base(args) {}

  [[nodiscard]]
  constexpr double &x() noexcept {
    return this->operator[](0);
  }
  [[nodiscard]]
  constexpr double &y() noexcept {
    return this->operator[](1);
  }
  [[nodiscard]]
  constexpr double &z() noexcept {
    return this->operator[](2);
  }
};

template <typename T> using Vec2 = blaze::StaticVector<T, 2UL>;

// For readability: So points look different than vectors
using Point3 = Vec3;

#endif