#include <cmath>
#include <cstddef>
#include <iostream>

#include "blaze/math/expressions/DVecMapExpr.h"
#include "blaze/math/expressions/DVecScalarMultExpr.h"
#include "color.h"
#include "hittable.h"
#include "hittable_list.h"
#include "ray.h"
#include "sphere.h"
#include "utility.h"
#include "vec.h"
#include <fpng.h>

Color ray_color(const Ray &ray, const Hittable &world) {
  HitRecord rec;
  if (world.hit(ray, 0, infinity, rec)) {
    return Color(0.5 * (rec.normal + Color{1, 1, 1}));
  }

  Vec3 unit_direction = Vec3(normalize(ray.direction()));
  auto a = 0.5 * (unit_direction.y() + 1.0);
  return Color((1.0 - a) * Color{1.0, 1.0, 1.0} + a * Color{0.5, 0.7, 1.0});
}

int main() {

  // -- Image --

  auto aspect_ratio = 16.0 / 9.0;
  auto img_dims = max(Vec2<size_t>{400, (size_t)(400 / aspect_ratio)}, 1U);

  // -- World --

  HittableList world;

  world.add(make_shared<Sphere>(Point3{0, 0, -1}, 0.5));
  world.add(make_shared<Sphere>(Point3{0, -100.5, -1}, 100));

  // -- Camera --

  auto focal_length = 1.0;
  auto viewport_h = 2.0; // 2 is arbitrary, can be any number

  Vec2<double> viewport_dims{
      viewport_h * (double(img_dims[0]) / img_dims[1]), viewport_h
  };
  Vec3 camera_center{0, 0, 0};

  // Calculate the vectors across the horizontal and down the vertical viewport
  // edges.
  auto viewport_u = Vec3{viewport_dims[0], 0, 0};
  auto viewport_v = Vec3{0, -viewport_dims[1], 0};

  // Calculate the horizontal and vertical delta vectors from pixel to pixel.
  auto pixel_delta_u = viewport_u / (double)img_dims[0];
  auto pixel_delta_v = viewport_v / (double)img_dims[1];

  // Calculate the location of the upper left pixel.
  auto viewport_upper_left = camera_center - Vec3{0, 0, focal_length} -
                             viewport_u / 2 - viewport_v / 2;
  auto pixel00_loc =
      viewport_upper_left + 0.5 * (pixel_delta_u + pixel_delta_v);

  // -- Render --

  std::cout << "P3\n" << img_dims[0] << ' ' << img_dims[1] << "\n255\n";

  for (size_t current_height = 0; current_height < img_dims[1];
       current_height++) {
    std::clog << "\rScanlines remaining: " << (img_dims[1] - current_height)
              << ' ' << std::flush;

    for (size_t current_width = 0; current_width < img_dims[0];
         current_width++) {
      auto pixel_center = pixel00_loc +
                          ((double)current_width * pixel_delta_u) +
                          ((double)current_height * pixel_delta_v);
      auto ray_direction = pixel_center - camera_center;
      Ray ray(camera_center, Vec3(ray_direction));

      Color pixel_color = ray_color(ray, world);
      write_color(std::cout, pixel_color);
    }
  }

  std::clog << "\rDone.                 \n";
}