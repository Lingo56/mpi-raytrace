#ifndef HITTABLE_LIST_H
#define HITTABLE_LIST_H

#include "hittable.h"
#include "interval.h"
#include "ray.h"

#include <concepts>
#include <memory>
#include <optional>
#include <type_traits>
#include <utility>
#include <vector>

class HittableList : public Hittable {
public:
  std::vector<std::unique_ptr<Hittable>> objects;

  constexpr void clear() noexcept { objects.clear(); }

  template <typename U>
    requires std::derived_from<U, Hittable>
  constexpr void add(U &&hittable) {
    objects.emplace_back(
        std::make_unique<std::remove_reference_t<U>>(std::forward<U>(hittable))
    );
  }

  [[nodiscard]] std::optional<HitRecord>
  hit(const Ray &ray, Interval<double> ray_t) const override {
    std::optional<HitRecord> result;
    auto closest_so_far = ray_t.max();

    for (const auto &object : objects) {
      auto record = object->hit(ray, Interval(ray_t.min(), closest_so_far));
      if (record.has_value()) {
        closest_so_far = record->time;
        result = std::move(*record);
      }
    }

    return result;
  }
};

#endif