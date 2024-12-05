#ifndef SPHERE_H
#define SPHERE_H

#include <algorithm>
#include <concepts>
#include <optional>
#include <utility>

#include "blaze/math/Vector.h"
#include "blaze/math/expressions/DVecNormExpr.h"

#include "hittable.h"
#include "interval.h"
#include "ray.h"
#include "vec.h"
#include <cmath>

class Sphere : public Hittable {
  Point3 sphere_center;
  double radius;

public:
  template <typename U>
    requires std::constructible_from<Point3, U>
  Sphere(U &&center, double radius)
      : sphere_center(std::forward<U>(center)), radius(std::max(0.0, radius)) {}

  std::optional<HitRecord>
  hit(const Ray &ray, Interval<double> ray_t) const override {
    Vec3 ray_to_center = Vec3(sphere_center - ray.origin());

    auto a_normal_ray_direction = sqrNorm(ray.direction());
    auto b_ray_to_center = dot(ray.direction(), ray_to_center);
    auto c_sphere_offset = sqrNorm(ray_to_center) - radius * radius;
    auto discriminant = (b_ray_to_center * b_ray_to_center) -
                        (a_normal_ray_direction * c_sphere_offset);
    if (discriminant < 0)
      return {};

    auto sqrtd = std::sqrt(discriminant);

    // Find the nearest root that lies in the acceptable range.
    auto root = (b_ray_to_center - sqrtd) / a_normal_ray_direction;
    if (!ray_t.surrounds(root)) {
      root = (b_ray_to_center + sqrtd) / a_normal_ray_direction;
      if (!ray_t.surrounds(root))
        return {};
    }

    return HitRecord::from_face_normal(
        ray, root, Vec3((ray.at(root) - sphere_center) / radius)
    );
  }
};

#endif