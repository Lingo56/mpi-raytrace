#ifndef VEC_H
#define VEC_H

#include "blaze/math/Vector.h"
#include "blaze/math/expressions/DVecScalarMultExpr.h"
#include "utility.h"
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

  constexpr Vec3(std::initializer_list<double> args) noexcept : Base(args) {}

  static Vec3 random(double min = -1, double max = 1) {
    return Vec3{random_vec<3, double>(min, max)};
  }

  static auto random_unit() { return blaze::normalize(random()); }

  static auto random_on_hemisphere(const Vec3 &normal) {
    auto on_unit_sphere = random_unit();
    // In same hemisphere as the normal
    double sign = std::copysign(1.0, dot(on_unit_sphere, normal));
    return on_unit_sphere * sign;
  }

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