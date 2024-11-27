#include <cmath>

#include "camera.h"
#include "hittable_list.h"
#include "sphere.h"
#include "utility.h"
#include "vec.h"
#include <fpng.h>

constexpr int image_width = 1280;
constexpr int image_height = 720;
constexpr int antialiasing_samples =
    100; // 1-8x for debug, 100x-500x for final render

int main() {
  // -- World --

  HittableList world;

  // Add spheres for "H"
  for (double i = -2; i <= 2; i++) {
    // Left vertical line
    world.add(make_shared<Sphere>(Point3{-2, i, -4}, 0.5));
    // Right vertical line
    world.add(make_shared<Sphere>(Point3{-0, i, -4}, 0.5));
  }
  for (double i = -1; i <= -0; i++) {
    // Horizontal connector
    world.add(make_shared<Sphere>(Point3{i, 0, -4}, 0.5));
  }

  // Add spheres for "I"
  for (double i = -2; i <= 2; i++) {
    // Vertical line
    world.add(make_shared<Sphere>(Point3{2, i, -4}, 0.5));
  }

  world.add(make_shared<Sphere>(Point3{0, -103, -1}, 100));

  camera cam(image_width, image_height, antialiasing_samples);

  cam.render(world);
}