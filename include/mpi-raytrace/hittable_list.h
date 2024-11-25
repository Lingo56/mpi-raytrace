#ifndef HITTABLE_LIST_H
#define HITTABLE_LIST_H

#include "hittable.h"
#include "ray.h"

#include <memory>
#include <vector>

using std::make_shared;
using std::shared_ptr;

class HittableList : public Hittable {
public:
  std::vector<shared_ptr<Hittable>> objects;

  HittableList() = default;
  explicit HittableList(const shared_ptr<Hittable> &object) { add(object); }

  void clear() { objects.clear(); }

  void add(const shared_ptr<Hittable> &object) { objects.push_back(object); }

  bool
  hit(const Ray &ray, double ray_tmin, double ray_tmax,
      HitRecord &rec) const override {
    HitRecord temp_rec;
    bool hit_anything = false;
    auto closest_so_far = ray_tmax;

    for (const auto &object : objects) {
      if (object->hit(ray, ray_tmin, closest_so_far, temp_rec)) {
        hit_anything = true;
        closest_so_far = temp_rec.t;
        rec = temp_rec;
      }
    }

    return hit_anything;
  }
};

#endif