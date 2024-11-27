#ifndef VEC_H
#define VEC_H

#include "blaze/math/Vector.h"
#include "blaze/math/expressions/DVecNormExpr.h"
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

  static Vec3 random() {
    return Vec3{random_double(), random_double(), random_double()};
  }

  static Vec3 random(double min, double max) {
    return Vec3{
        random_double(min, max),
        random_double(min, max),
        random_double(min, max)
    };
  }

  static Vec3 random_unit_vector() {
    while (true) {
      // Generate a random vector in [-1, 1]
      Vec3 random_vector{
          random_double(-1.0, 1.0),
          random_double(-1.0, 1.0),
          random_double(-1.0, 1.0)
      };

      // Calculate squared length
      double len_sqr = sqrNorm(random_vector);

      // Check if the point lies inside the unit sphere
      if (len_sqr <= 1.0) {
        if (1e-160 < len_sqr && len_sqr <= 1)
          return Vec3(random_vector / std::sqrt(len_sqr));
      }
    }
  }

  static Vec3 random_on_hemisphere(const Vec3 &normal) {
    Vec3 on_unit_sphere = random_unit_vector();
    if (dot(on_unit_sphere, normal) > 0.0) // In same hemisphere as the normal
      return on_unit_sphere;
    else
      return Vec3(-on_unit_sphere);
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