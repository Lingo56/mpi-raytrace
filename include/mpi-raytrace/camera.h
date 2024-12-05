#ifndef CAMERA_H
#define CAMERA_H

#include <atomic>
#include <chrono>
#include <cstddef>
#include <functional>
#include <future>
#include <iostream>
#include <ostream>
#include <ranges>
#include <stdexcept>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include "color.h"
#include "hittable.h"
#include "interval.h"
#include "ray.h"
#include "vec.h"

#ifdef __cpp_lib_hardware_interference_size
using std::hardware_constructive_interference_size;
using std::hardware_destructive_interference_size;
#else
// 64 bytes on x86-64
constexpr std::size_t hardware_constructive_interference_size = 64;
constexpr std::size_t hardware_destructive_interference_size = 64;
#endif

class Camera {
  alignas(hardware_destructive_interference_size
  ) std::atomic<size_t> rows_completed =
      0;                        // Counter for current render progress
  double aspect_ratio;          // Ratio of image width and height
  Vec2<size_t> img_dims;        // Rendered image dimensions
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
                        (((double)current_width + offset.x()) * pixel_delta_u) +
                        (((double)current_height + offset.y()) * pixel_delta_v);

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
    auto coeff_a = 0.5 * (unit_direction.y() + 1.0);
    return Color{
        (1.0 - coeff_a) * Color{1.0, 1.0, 1.0} + coeff_a * Color{0.5, 0.7, 1.0}
    };
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
      const Hittable &world, Interval<size_t> work_interval, size_t width,
      std::vector<std::vector<Color>> &image
  ) {
    auto start_row = work_interval.begin();
    auto end_row = work_interval.end();

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
      rows_completed.fetch_add(1, std::memory_order_acq_rel);
    }
  }

  void render(const Hittable &world, size_t total_threads) { // -- Render --
    const size_t width = img_dims[0];
    const size_t height = img_dims[1];
    std::vector<std::vector<Color>> image(height, std::vector<Color>(width));

    size_t rows_per_thread = height / total_threads;
    std::vector<std::future<void>> futures;

    for (size_t thread_idx = 0; thread_idx < total_threads; ++thread_idx) {
      size_t start_row = thread_idx * rows_per_thread;
      size_t end_row = (thread_idx == total_threads - 1)
                           ? height
                           : start_row + rows_per_thread;

      futures.emplace_back(std::async(
          std::launch::async,
          [&]<typename... Args>(Args &&...args) {
            this->render_chunk(std::forward<Args>(args)...);
          },
          std::ref(world),
          Interval<size_t>{start_row, end_row},
          width,
          std::ref(image)
      ));
    }

    // NOLINTNEXTLINE(cppcoreguidelines-avoid-do-while)
    do {
      std::this_thread::sleep_for(std::chrono::milliseconds(2));

      size_t progress =
          rows_completed.load(std::memory_order_acq_rel) * 100 / img_dims[1];
      size_t bar_width = 50; // Width of the progress bar in characters
      size_t pos = (progress * bar_width) / 100;

      std::string bar =
          "[" + std::string(pos, '=') + std::string(bar_width - pos, ' ') + "]";
      std::clog << "\r" << "\x1B[2K" << bar << " " << progress << "%"
                << std::flush;
    } while (!std::ranges::empty(
        futures | std::views::filter([](const auto &future) {
          switch (future.wait_for(std::chrono::milliseconds(0))) {
          case std::future_status::ready:
            return false;
          case std::future_status::timeout:
            return true;
          case std::future_status::deferred:
            throw std::logic_error("impossible");
          }
        })
    ));
    std::clog << "\n";

    for (auto &future : futures) {
      future.wait();
    }

    // Output the image after all threads finish
    std::cout << "P3\n" << width << ' ' << height << "\n255\n";
    for (const auto &row : image) {
      for (const auto &pixel : row) {
        write_color(std::cout, pixel);
      }
    }

    std::clog << "Done.\n";
  }
};

#endif