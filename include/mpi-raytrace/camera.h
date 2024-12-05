#ifndef CAMERA_H
#define CAMERA_H

#include <algorithm>
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
  ) std::atomic<size_t> rows_completed = 0; // Counter for render progress
  double aspect_ratio;                      // Ratio of image width and height
  Vec2<size_t> img_dims;                    // Rendered image dimensions
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

  // Returns the vector to a random point in the [-.5,-.5]-[+.5,+.5] unit
  // square.
  [[nodiscard]] static auto sample_square() {
    auto res = Vec3::random(-0.5, 0.5);
    res.z() = 0;
    return res;
  }

  [[gnu::hot]] [[nodiscard]]
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

    return {ray_origin, ray_direction};
  }

  [[nodiscard]]
  static Color ray_color_helper(
      const Ray &ray, size_t depth, const Hittable &world, double attenuation
  ) {
    constexpr auto EPSILON = 0.001; // shadow acne fix

    if (depth == 0)
      return Color{0, 0, 0};

    auto rec = world.hit(ray, Interval(EPSILON, infinity));

    if (rec.has_value()) {
      Vec3 direction = Vec3(rec->normal + Vec3::random_unit());
      [[clang::musttail]] return ray_color_helper(
          Ray(rec->point, direction), depth - 1, world, attenuation * 0.7
      );
    }

    Vec3 unit_direction = Vec3{blaze::normalize(ray.direction())};
    auto coeff_a = 0.5 * (unit_direction.y() + 1.0);
    return Color{
        (1.0 - coeff_a) * Color{1.0, 1.0, 1.0} + coeff_a * Color{0.5, 0.7, 1.0}
    };
  }

  // Trace a ray through a world with a maximum depth.
  [[gnu::hot]] [[nodiscard]]
  static Color ray_color(const Ray &ray, size_t depth, const Hittable &world) {
    return ray_color_helper(ray, depth, world, 1.0);
  }

  // This function now continuously obtains work in 256-row chunks
  // until all rows are processed.
  void render_thread(
      const Hittable &world, size_t width, size_t height,
      std::atomic<size_t> &next_row, std::vector<std::vector<Color>> &image
  ) {
    constexpr size_t CHUNK_SIZE = 1;

    for (;;) {
      size_t start = next_row.fetch_add(CHUNK_SIZE, std::memory_order_acq_rel);
      if (start >= height)
        break;
      size_t end = std::min(start + CHUNK_SIZE, height);

      // Go through each pixel in the image one by one,
      // generate a random ray that originates from the pixel,
      // and trace it.
      for (size_t current_height = start; current_height < end;
           ++current_height) {
        for (size_t current_width = 0; current_width < width; ++current_width) {
          Color pixel_color{0, 0, 0};
          for (size_t sample = 0; sample < rays_per_pixel; ++sample) {
            Ray ray = get_ray(current_width, current_height);
            pixel_color += ray_color(ray, max_bounces, world);
          }
          // Store the result
          image[current_height][current_width] =
              Color{pixel_color * pixel_samples_scale};
        }
      }

      // Update progress after finishing a row for progress bar.
      rows_completed.fetch_add(end - start, std::memory_order_acq_rel);
    }
  }

public:
  Camera(
      double image_width, double image_height, size_t samples_per_pixel,
      size_t max_bounces
  )
      : aspect_ratio(image_width / image_height),
        rays_per_pixel(samples_per_pixel), max_bounces(max_bounces) {
    img_dims = blaze::max(
        Vec2<size_t>{
            (size_t)(image_height), (size_t)(image_height / aspect_ratio)
        },
        1U
    );

    initialize();
  }

  // Renders a `world` through this camera.
  void render(const Hittable &world, size_t total_threads) {
    const size_t width = img_dims[0];
    const size_t height = img_dims[1];
    std::vector<std::vector<Color>> image(height, std::vector<Color>(width));

    std::atomic<size_t> next_row(0);

    std::vector<std::future<void>> futures;
    futures.reserve(total_threads);

    auto start_time = std::chrono::steady_clock::now();

    // Spawn threads
    for (size_t thread_idx = 0; thread_idx < total_threads; ++thread_idx) {
      futures.emplace_back(std::async(
          std::launch::async,
          // lambda to bind `this`.
          [&]<typename... Args>(Args &&...args) {
            this->render_thread(std::forward<Args>(args)...);
          },
          std::ref(world),
          width,
          height,
          std::ref(next_row),
          std::ref(image)
      ));
    }

    // Render the progress bar every 2ms in a loop until all threads finish.
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-do-while)
    do {
      std::this_thread::sleep_for(std::chrono::milliseconds(2));

      size_t progress =
          rows_completed.load(std::memory_order_acquire) * 100 / height;
      size_t bar_width = 50; // Width of the progress bar in characters
      size_t pos = (progress * bar_width) / 100;

      std::chrono::duration<double> elapsed =
          std::chrono::steady_clock::now() - start_time;

      std::string bar =
          "[" + std::string(pos, '=') + std::string(bar_width - pos, ' ') + "]";
      std::clog << "\r" << "\x1B[2K" << bar << " " << progress << "% "
                << elapsed.count() << " seconds" << std::flush;
    } while (!std::ranges::empty(
        futures | std::views::filter([](const auto &future) {
          switch (future.wait_for(std::chrono::milliseconds(0))) {
          case std::future_status::ready:
            return false;
          case std::future_status::timeout:
            return true;
          case std::future_status::deferred:
            // Impossible due to creating futures from std::async with
            // std::launch::async
            throw std::logic_error("impossible");
          }
        })
    ));
    std::clog << "\n";

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