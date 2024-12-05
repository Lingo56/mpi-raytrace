// Source for core logic:
// https://raytracing.github.io/books/RayTracingInOneWeekend.html

#include <cmath>
#include <cstddef>

#include "camera.h"
#include "hittable_list.h"
#include "sphere.h"
#include "vec.h"

constexpr int image_width = 1280;
constexpr int image_height = 720;
constexpr int rays_per_pixel = 200; // 1-8x debug, 100x-500x final render
constexpr int max_bounces = 6;      // Max times rays can bounce. Default: 10
                                    // Lower = faster but less accurate

constexpr int thread_count = 12; // Number of CPU threads the renderer will use

int main() {
  // -- World --

  HittableList world;

  // Add spheres for "H"
  for (size_t i = -2; i <= 2; i++) {
    // Left vertical line
    // world.add(make_shared<Sphere>(Point3{-2, i, -4}, 0.5));
    world.add(Sphere{Point3{-2, (double)i, -4}, 0.5});
    // Right vertical line
    world.add(Sphere{Point3{-0, (double)i, -4}, 0.5});
  }
  for (size_t j = -1; j <= -0; j++) {
    // Horizontal connector
    world.add(Sphere{Point3{(double)j, 0, -4}, 0.5});
  }

  // Add spheres for "I"
  for (size_t i = -2; i <= 2; i++) {
    // Vertical line
    world.add(Sphere{Point3{2, (double)i, -4}, 0.5});
  }

  world.add(Sphere{Point3{0, -102.5, -1}, 100});

  camera cam(image_width, image_height, rays_per_pixel, max_bounces);

  cam.render(world, thread_count);
}