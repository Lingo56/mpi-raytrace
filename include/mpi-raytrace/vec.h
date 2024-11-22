#ifndef VEC_H
#define VEC_H

#include <cmath>

#include <blaze/Blaze.h>
#include <blaze/math/dense/StaticVector.h>
#include <concepts>
#include <initializer_list>

// using Vec3 = blaze::StaticVector<double, 3UL>;
struct Vec3 : public blaze::StaticVector<double, 3UL> {
  using Base = blaze::StaticVector<double, 3UL>;

  template <typename U>
    requires std::constructible_from<Base, U>
  constexpr Vec3(U &&arg) noexcept : Base(std::forward<U>(arg)) {}

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

#endif