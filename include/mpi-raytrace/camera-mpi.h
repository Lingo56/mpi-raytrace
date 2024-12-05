#ifndef CAMERA_H
#define CAMERA_H

#include <cstddef>
#include <iostream>
#include <ostream>
#include <vector>

#include <mpi.h>

#include "color.h"
#include "hittable.h"
#include "interval.h"
#include "ray.h"
#include "utility.h"
#include "vec.h"

class Camera {
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

  [[nodiscard]] Ray get_ray(size_t current_width, size_t current_height) {
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

      [[clang::musttail]] return ray_color_helper(
          Ray(rec->point, direction), depth - 1, world, attenuation * 0.7
      );
    }

    Vec3 unit_direction = Vec3(blaze::normalize(ray.direction()));
    auto coeff_a = 0.5 * (unit_direction.y() + 1.0);
    return Color{
        (1.0 - coeff_a) * Color{1.0, 1.0, 1.0} + coeff_a * Color{0.5, 0.7, 1.0}
    };
  }

  // Trace a ray through a world with a maximum depth.
  [[nodiscard]] static Color
  ray_color(const Ray &ray, size_t depth, const Hittable &world) {
    return ray_color_helper(ray, depth, world, 1.0);
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

  // Entrypoint for processes.
  void render_chunk(
      const Hittable &world, Interval<size_t> work_interval, size_t width,
      std::vector<std::vector<Color>> &image
  ) {
    auto start_row = work_interval.begin();
    auto end_row = work_interval.end();
    size_t local_height = end_row - start_row;

    // Go through each pixel in the image one by one,
    // generate a random ray that originates from the pixel,
    // and trace it.
    for (size_t local_row = 0; local_row < local_height; ++local_row) {
      size_t current_height = start_row + local_row;
      for (size_t current_width = 0; current_width < width; ++current_width) {
        Color pixel_color{0, 0, 0};
        for (size_t sample = 0; sample < rays_per_pixel; ++sample) {
          Ray ray = get_ray(current_width, current_height);
          pixel_color += ray_color(ray, max_bounces, world);
        }
        // Store the result
        image[local_row][current_width] =
            Color(pixel_color * pixel_samples_scale);
      }
    }
  }

  // Renders a `world` through this camera.
  void render(const Hittable &world) {
    MPI_Init(nullptr, nullptr);

    int rank_{}, size_{};
    MPI_Comm_rank(MPI_COMM_WORLD, &rank_);
    MPI_Comm_size(MPI_COMM_WORLD, &size_);

    auto rank = try_narrow<size_t>(rank_);
    auto size = try_narrow<size_t>(size_);

    const size_t width = img_dims[0];
    const size_t height = img_dims[1];

    size_t rows_per_process = height / size;
    size_t remainder_rows = height % size;

    // Calculate work partition for self.
    size_t start_row{}, end_row{};
    if (rank < remainder_rows) {
      start_row = rank * (rows_per_process + 1);
      end_row = start_row + rows_per_process + 1;
    } else {
      start_row = rank * rows_per_process + remainder_rows;
      end_row = start_row + rows_per_process;
    }

    size_t local_height = end_row - start_row;

    // Each process creates local image buffer
    std::vector<std::vector<Color>> local_image(
        local_height, std::vector<Color>(width)
    );

    auto start_time = std::chrono::steady_clock::now();

    // Each process renders its chunk
    this->render_chunk(world, Interval{start_row, end_row}, width, local_image);

    // Each process flattens local_image into send_buffer
    size_t num_elements = local_height * width * 3;
    std::vector<double> send_buffer(num_elements);

    // Copy data from local_image to send_buffer
    for (size_t i = 0; i < local_height; ++i) {
      for (size_t j = 0; j < width; ++j) {
        size_t idx = (i * width + j) * 3;
        send_buffer[idx] = local_image[i][j].x();
        send_buffer[idx + 1] = local_image[i][j].y();
        send_buffer[idx + 2] = local_image[i][j].z();
      }
    }

    // Rank 0 sets up receive buffer and counts/displacements
    std::vector<int> recvcounts(size);
    std::vector<int> displs(size);

    size_t total_elements = height * width * 3;
    std::vector<double> recv_buffer;

    if (rank == 0) {
      recv_buffer.resize(total_elements);
    }

    // Compute recvcounts and displs on all ranks
    size_t offset = 0;
    for (size_t i = 0; i < size; ++i) {
      size_t proc_rows =
          (i < remainder_rows) ? (rows_per_process + 1) : rows_per_process;
      size_t proc_elements = proc_rows * width * 3;
      recvcounts[i] = static_cast<int>(proc_elements);
      displs[i] = static_cast<int>(offset);
      offset += proc_elements;
    }

    // Now, gather data from all processes
    MPI_Gatherv(
        send_buffer.data(),
        static_cast<int>(num_elements),
        MPI_DOUBLE,
        recv_buffer.data(),
        recvcounts.data(),
        displs.data(),
        MPI_DOUBLE,
        0,
        MPI_COMM_WORLD
    );

    if (rank == 0) {
      // After gathering, reconstruct the image
      std::vector<std::vector<Color>> image(height, std::vector<Color>(width));

      size_t idx = 0;
      for (size_t i = 0; i < height; ++i) {
        for (size_t j = 0; j < width; ++j) {
          image[i][j] = Color{
              recv_buffer[idx], recv_buffer[idx + 1], recv_buffer[idx + 2]
          };
          idx += 3;
        }
      }

      std::chrono::duration<double> elapsed =
          std::chrono::steady_clock::now() - start_time;

      // Output the image after all processes finish
      std::cout << "P3\n" << width << ' ' << height << "\n255\n";
      for (const auto &row : image) {
        for (const auto &pixel : row) {
          write_color(std::cout, pixel);
        }
      }

      std::clog << "Done in " << elapsed.count() << " seconds.";
    }

    MPI_Finalize();
  }
};

#endif
