// Source for core logic:
// https://raytracing.github.io/books/RayTracingInOneWeekend.html

#include <cmath>
#include <cstddef>
#include <iostream>
#include <string>
#include <thread>

#include <cxxopts.hpp>
#include <fmt/format.h>

#ifdef USE_MPI
#include "camera-mpi.h"
#else
#include "camera.h"
#endif

#include "hittable_list.h"
#include "sphere.h"
#include "vec.h"

HittableList build_world() {
  HittableList world;

  // Add spheres for "H"
  for (std::ptrdiff_t i = -2; i <= 2; i++) {
    // Left vertical line
    // world.add(make_shared<Sphere>(Point3{-2, i, -4}, 0.5));
    world.add(Sphere{Point3{-2, (double)i, -4}, 0.5});
    // Right vertical line
    world.add(Sphere{Point3{-0, (double)i, -4}, 0.5});
  }
  for (std::ptrdiff_t j = -1; j <= -0; j++) {
    // Horizontal connector
    world.add(Sphere{Point3{(double)j, 0, -4}, 0.5});
  }

  // Add spheres for "I"
  for (std::ptrdiff_t i = -2; i <= 2; i++) {
    // Vertical line
    world.add(Sphere{Point3{2, (double)i, -4}, 0.5});
  }

  world.add(Sphere{Point3{0, -102.5, -1}, 100});

  return world;
};

int main(int argc, char **argv) {
  using namespace cxxopts;

  Options options{"raytrace", "Render a scene using ray tracing."};
  options
      .add_options()("w,width", "Width of image output in pixels.", cxxopts::value<size_t>()->default_value("1920"))("h,height", "Height of image output in pixels.", cxxopts::value<size_t>()->default_value("1080"))("r,rays", "Number of rays shot out per pixel.", cxxopts::value<size_t>()->default_value("32"))(
          "b,bounce",
          "Maximum number of times rays can bounce. Lower is faster but less accurate.",
          cxxopts::value<size_t>()->default_value("4")
      )("t,threads", "Number of threads to use. Default is auto-detected from the CPU.", cxxopts::value<size_t>()->default_value(std::to_string(std::thread::hardware_concurrency())));

  auto args = options.parse(argc, argv);

  auto image_width = args["width"].as<size_t>();
  auto image_height = args["height"].as<size_t>();
  auto rays_per_pixel = args["rays"].as<size_t>();
  auto max_bounces = args["bounce"].as<size_t>();
  auto n_threads = args["threads"].as<size_t>();

  std::clog << fmt::format(
      "Rendering a {}x{}px image with {} rays/px and {} max bounces.\n",
      image_width,
      image_height,
      rays_per_pixel,
      max_bounces
  );
#ifndef USE_MPI
  std::clog << fmt::format("Using {} threads.\n", n_threads);
#endif

  Camera cam(
      (double)image_width, (double)image_height, rays_per_pixel, max_bounces
  );
  auto world = build_world();

#ifdef USE_MPI
  cam.render(world);
#else
  cam.render(world, n_threads);
#endif
}