#ifndef CAMERA_H
#define CAMERA_H

#include "blaze/math/expressions/DVecMapExpr.h"
#include "blaze/math/expressions/DVecScalarMultExpr.h"
#include "color.h"
#include "hittable.h"
#include "hittable_list.h"
#include "interval.h"
#include "ray.h"
#include "sphere.h"
#include "utility.h"
#include "vec.h"
#include <cstddef>
#include <iostream>
#include <ostream>

class camera {
public:
  double aspect_ratio = 16.0 / 9.0;

  camera(double image_width, double image_height)
      : aspect_ratio(image_width / image_height) {
    img_dims = max(
        Vec2<size_t>{
            static_cast<unsigned long>(image_height),
            (size_t)(image_height / aspect_ratio)
        },
        1U
    );
    initialize();
  }

  void render(const Hittable &world) { // -- Render --

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

private:
  Vec2<size_t> img_dims; // Rendered image dimensions
  Point3 camera_center;  // Camera center
  Point3 pixel00_loc;    // Location of pixel 0, 0
  Vec3 pixel_delta_u;    // Offset to pixel to the right
  Vec3 pixel_delta_v;    // Offset to pixel below

  void initialize() {
    auto focal_length = 1.0;
    auto viewport_h = 2.0; // 2 is arbitrary, can be any number

    Vec2<double> viewport_dims{
        viewport_h * (double(img_dims[0]) / double(img_dims[1])), viewport_h
    };

    camera_center = Point3({0, 0, 0});

    // Calculate the vectors across the horizontal and down the vertical
    // viewport edges.
    auto viewport_u = Vec3{viewport_dims[0], 0, 0};
    auto viewport_v = Vec3{0, -viewport_dims[1], 0};

    // Calculate the horizontal and vertical delta vectors from pixel to pixel.
    pixel_delta_u = Vec3(viewport_u / (double)img_dims[0]);
    pixel_delta_v = Vec3(viewport_v / (double)img_dims[1]);

    // Calculate the location of the upper left pixel.
    auto viewport_upper_left = camera_center - Vec3{0, 0, focal_length} -
                               viewport_u / 2 - viewport_v / 2;

    pixel00_loc =
        Point3(viewport_upper_left + 0.5 * (pixel_delta_u + pixel_delta_v));
  }

  static Color ray_color(const Ray &ray, const Hittable &world) {
    HitRecord rec;
    if (world.hit(ray, Interval(0, infinity), rec)) {
      return Color(0.5 * (rec.normal + Color{1, 1, 1}));
    }

    Vec3 unit_direction = Vec3(normalize(ray.direction()));
    auto a = 0.5 * (unit_direction.y() + 1.0);
    return Color((1.0 - a) * Color{1.0, 1.0, 1.0} + a * Color{0.5, 0.7, 1.0});
  }
};

#endif