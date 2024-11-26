#ifndef HITTABLE_H
#define HITTABLE_H

#include "ray.h"
#include "vec.h"

// Holds info about where the ray had a collision
class HitRecord {
public:
  Point3 p;
  Vec3 normal;
  double t{};
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

  // Only counts rays between tmin and tmax
  virtual bool
  hit(const Ray &ray, double ray_tmin, double ray_tmax,
      HitRecord &rec) const = 0;
};

#endif