#ifndef HITTABLE_H
#define HITTABLE_H

#include "blaze/math/Vector.h"
#include "interval.h"
#include "ray.h"
#include "vec.h"
#include <cmath>
#include <optional>

// Holds info about where the ray had a collision
struct HitRecord {
  Point3 point;
  // normal is in relation to surface orientation.
  Vec3 normal;
  double time{};
  // is the hit on the front or back of the surface?
  bool is_frontface{};

  // Creates a `HitRecord` based on the vector point away from the surface's
  // outer side, aka the `outward_normal`.
  [[nodiscard]]
  static HitRecord
  from_face_normal(const Ray &ray, double time, const Vec3 &outward_normal) {
    // NOTE: the parameter `outward_normal` is assumed to have unit length.
    auto is_frontface = dot(ray.direction(), outward_normal) < 0;

    return {
        ray.at(time),
        Vec3{outward_normal * std::copysign(1.0, is_frontface)},
        time,
        is_frontface
    };
  }
};

// For objects that rays can hit
class Hittable {
public:
  virtual ~Hittable() = default;
  Hittable() = default;

  Hittable(const Hittable &) = default;
  Hittable(Hittable &&) = default;

  Hittable &operator=(const Hittable &) = default;
  Hittable &operator=(Hittable &&) = default;

  // Only counts rays between Interval [tmin, tmax]
  [[nodiscard]]
  virtual std::optional<HitRecord>
  hit(const Ray &ray, Interval<double> ray_t) const = 0;
};

#endif