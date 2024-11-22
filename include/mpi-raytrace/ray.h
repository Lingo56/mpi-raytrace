#ifndef RAY_H
#define RAY_H

#include "vec.h"

struct Ray {
  [[nodiscard]] const Vec3 &origin() const { return orig; }
  [[nodiscard]] const Vec3 &direction() const { return dir; }

  [[nodiscard]] Vec3 at(double time) const { return orig + time * dir; }

  Vec3 orig;
  Vec3 dir;
};

#endif