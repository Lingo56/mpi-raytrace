#ifndef CAMERA_H
#define CAMERA_H

#include <atomic>
#include <cstddef>
#include <functional>
#include <iostream>
#include <ostream>
#include <stop_token>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include "blaze/math/expressions/DVecMapExpr.h"
#include "blaze/math/expressions/DVecScalarMultExpr.h"

#include "color.h"
#include "hittable.h"
#include "interval.h"
#include "ray.h"
#include "vec.h"

class Camera {
  std::atomic<size_t> rows_completed = 0; // Counter for current render progress
  double aspect_ratio;                    // Ratio of image width and height
  Vec2<size_t> img_dims;                  // Rendered image dimensions
  size_t rays_per_pixel;        // Anti-aliasing sample count for each pixel
  double pixel_samples_scale{}; // Color scale factor for a sum of pixel samples
  size_t max_bounces;           // The max times rays can bounce in the scene

  Point3 camera_center; // Camera center
  Point3 pixel00_loc;   // Location of pixel 0, 0
  Vec3 pixel_delta_u;   // Offset to pixel to the right
  Vec3 pixel_delta_v;   // Offset to pixel below

  void initialize() {
    auto focal_length = 1.0;
    auto viewport_h = 2.0; // 2 is arbitrary, can be any number

    pixel_samples_scale = 1.0 / (double)rays_per_pixel;

    Vec2<double> viewport_dims{
        viewport_h * (double(img_dims[0]) / double(img_dims[1])), viewport_h
    };

    camera_center = Point3({0, 0, 0});

    // Calculate the vectors across the horizontal and down the vertical
    // viewport edges.
    auto viewport_u = Vec3{viewport_dims[0], 0, 0};
    auto viewport_v = Vec3{0, -viewport_dims[1], 0};

    // Calculate the horizontal and vertical delta vectors from pixel to pixel.
    pixel_delta_u = Vec3{viewport_u / (double)img_dims[0]};
    pixel_delta_v = Vec3{viewport_v / (double)img_dims[1]};

    // Calculate the location of the upper left pixel.
    auto viewport_upper_left = Vec3{
        camera_center - Vec3{0, 0, focal_length} - viewport_u / 2 -
        viewport_v / 2
    };

    pixel00_loc =
        Point3(viewport_upper_left + 0.5 * (pixel_delta_u + pixel_delta_v));
  }

  [[nodiscard]] static auto sample_square() {
    // Returns the vector to a random point in the [-.5,-.5]-[+.5,+.5] unit
    // square.
    auto res = Vec3::random(-0.5, 0.5);
    res.z() = 0;
    return res;
  }

  [[nodiscard]]
  Ray get_ray(size_t current_width, size_t current_height) {
    // Construct a camera ray originating from the origin and directed at
    // randomly sampled
    // point around the pixel location i, j.

    auto offset = sample_square();
    auto pixel_sample = pixel00_loc +
                        ((current_width + offset.x()) * pixel_delta_u) +
                        ((current_height + offset.y()) * pixel_delta_v);

    auto ray_origin = camera_center;
    auto ray_direction = Vec3{pixel_sample - ray_origin};

    return {ray_origin, Vec3(ray_direction)};
  }

  [[nodiscard]]
  // NOLINTNEXTLINE(misc-no-recursion) - OK because of musttail
  static Color ray_color_helper(
      const Ray &ray, size_t depth, const Hittable &world, double attenuation
  ) {
    constexpr auto EPSILON = 0.001; // shadow acne fix

    if (depth <= 0)
      return Color{0, 0, 0};

    auto rec = world.hit(ray, Interval(EPSILON, infinity));

    if (rec.has_value()) {
      Vec3 direction = Vec3(rec->normal + Vec3::random_unit());

      __attribute__((musttail)) return ray_color_helper(
          Ray(rec->point, direction), depth - 1, world, attenuation * 0.7
      );
    }

    Vec3 unit_direction = Vec3(normalize(ray.direction()));
    auto a = 0.5 * (unit_direction.y() + 1.0);
    return Color{(1.0 - a) * Color{1.0, 1.0, 1.0} + a * Color{0.5, 0.7, 1.0}};
  }

  [[nodiscard]]
  static Color ray_color(const Ray &ray, size_t depth, const Hittable &world) {
    return ray_color_helper(ray, depth, world, 1.0);
  }

public:
  Camera(
      double image_width, double image_height, size_t samples_per_pixel,
      size_t max_bounces
  )
      : aspect_ratio(image_width / image_height),
        rays_per_pixel(samples_per_pixel), max_bounces(max_bounces) {
    img_dims = max(
        Vec2<size_t>{
            (size_t)(image_height), (size_t)(image_height / aspect_ratio)
        },
        1U
    );

    initialize();
  }

  void render_chunk(
      const Hittable &world, size_t start_row, size_t end_row, size_t width,
      std::vector<std::vector<Color>> &image
  ) {
    for (size_t current_height = start_row; current_height < end_row;
         ++current_height) {
      for (size_t current_width = 0; current_width < width; ++current_width) {
        Color pixel_color{0, 0, 0};
        for (size_t sample = 0; sample < rays_per_pixel; ++sample) {
          Ray ray = get_ray(current_width, current_height);
          pixel_color += ray_color(ray, max_bounces, world);
        }
        // Store the result
        image[current_height][current_width] =
            Color(pixel_color * pixel_samples_scale);
      }

      // Update progress after finishing a row
      ++rows_completed;
      size_t progress =
          rows_completed.load(std::memory_order_acq_rel) * 100 / img_dims[1];
      size_t bar_width = 50; // Width of the progress bar in characters
      size_t pos = (progress * bar_width) / 100;

      std::string bar =
          "[" + std::string(pos, '=') + std::string(bar_width - pos, ' ') + "]";
      std::clog << "\r" << bar << " " << progress << "% " << std::flush;
    }
  }

  void render(const Hittable &world, size_t thread_count) { // -- Render --
    const size_t width = img_dims[0];
    const size_t height = img_dims[1];
    std::vector<std::vector<Color>> image(height, std::vector<Color>(width));

    size_t rows_per_thread = height / thread_count;
    std::vector<std::jthread> pool;
    pool.reserve(thread_count);

    for (size_t thread = 0; thread < thread_count; ++thread) {
      size_t start_row = thread * rows_per_thread;
      size_t end_row =
          (thread == thread_count - 1) ? height : start_row + rows_per_thread;

      pool.emplace_back(
          [&]<typename... Args>(std::stop_token, Args &&...args) {
            this->render_chunk(std::forward<Args>(args)...);
          },
          std::ref(world),
          start_row,
          end_row,
          width,
          std::ref(image)
      );
    }

    for (auto &thread : pool) {
      thread.join();
    }

    // Output the image after all threads finish
    std::cout << "P3\n" << width << ' ' << height << "\n255\n";
    for (const auto &row : image) {
      for (const auto &pixel : row) {
        write_color(std::cout, pixel);
      }
    }

    std::clog << "\rDone.                 \n";
  }
};

#endif