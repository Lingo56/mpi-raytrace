#ifndef RAY_H
#define RAY_H

#include <utility>

#include "vec.h"

struct Ray {
public:
  Ray() = default;

  Ray(Point3 origin, Vec3 direction)
      : orig(std::move(origin)), dir(std::move(direction)) {}

  [[nodiscard]] const Vec3 &origin() const { return orig; }
  [[nodiscard]] const Vec3 &direction() const { return dir; }

  [[nodiscard]] Vec3 at(double time) const { return Vec3(orig + time * dir); }

private:
  Vec3 orig;
  Vec3 dir;
};

#endif