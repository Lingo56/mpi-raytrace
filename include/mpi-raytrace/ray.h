#ifndef RAY_H
#define RAY_H

#include <concepts>
#include <utility>

#include "vec.h"

struct Ray {
  Point3 origin_;
  Vec3 direction_;

public:
  template <typename O, typename D>
    requires std::constructible_from<Point3, O> &&
                 std::constructible_from<Vec3, D>
  constexpr Ray(O &&origin, D &&direction)
      : origin_(std::forward<O>(origin)),
        direction_(std::forward<D>(direction)) {}

  [[nodiscard]] constexpr const Point3 &origin() const { return origin_; }
  [[nodiscard]] constexpr const Vec3 &direction() const { return direction_; }

  [[nodiscard]] auto at(double time) const {
    return Point3{origin_ + time * direction_};
  }
};

#endif