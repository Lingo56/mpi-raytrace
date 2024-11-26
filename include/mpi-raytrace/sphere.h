#ifndef SPHERE_H
#define SPHERE_H

#include "blaze/math/Vector.h"
#include <utility>

#include "blaze/math/expressions/DVecNormExpr.h"
#include "hittable.h"
#include "ray.h"
#include "vec.h"
#include <cmath>

class Sphere : public Hittable {
public:
  Sphere(Point3 center, double radius)
      : sphere_center(std::move(center)), radius(std::fmax(0, radius)) {}

  bool hit(const Ray &ray, Interval ray_t, HitRecord &rec) const override {
    Vec3 ray_to_center = Vec3(sphere_center - ray.origin());

    auto a_normal_ray_direction = sqrNorm(ray.direction());
    auto b_ray_to_center = dot(ray.direction(), ray_to_center);
    auto c_sphere_offset = sqrNorm(ray_to_center) - radius * radius;
    auto discriminant = (b_ray_to_center * b_ray_to_center) -
                        (a_normal_ray_direction * c_sphere_offset);
    if (discriminant < 0)
      return false;

    auto sqrtd = std::sqrt(discriminant);

    // Find the nearest root that lies in the acceptable range.
    auto root = (b_ray_to_center - sqrtd) / a_normal_ray_direction;
    if (!ray_t.surrounds(root)) {
      root = (b_ray_to_center + sqrtd) / a_normal_ray_direction;
      if (!ray_t.surrounds(root))
        return false;
    }

    rec.t = root;
    rec.p = ray.at(rec.t);
    Vec3 outward_normal = Vec3((rec.p - sphere_center) / radius);
    rec.set_face_normal(ray, outward_normal);

    return true;
  }

private:
  Point3 sphere_center;
  double radius;
};

#endif