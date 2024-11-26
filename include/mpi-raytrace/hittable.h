#ifndef HITTABLE_H
#define HITTABLE_H

#include "blaze/math/Vector.h"
#include "ray.h"
#include "vec.h"

// Holds info about where the ray had a collision
class HitRecord {
public:
  Point3 p;
  Vec3 normal;
  double t{};
  bool front_face{};

  void set_face_normal(const Ray &ray, const Vec3 &outward_normal) {
    // Sets the hit record normal vector.
    // NOTE: the parameter `outward_normal` is assumed to have unit length.

    front_face = dot(ray.direction(), outward_normal) < 0;
    normal = front_face ? outward_normal : Vec3(-outward_normal);
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

  // Only counts rays between tmin and tmax
  virtual bool
  hit(const Ray &ray, double ray_tmin, double ray_tmax,
      HitRecord &rec) const = 0;
};

#endif