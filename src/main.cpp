#include <cmath>
#include <cstddef>
#include <iostream>

#include "blaze/math/expressions/DVecMapExpr.h"
#include "blaze/math/expressions/DVecScalarMultExpr.h"
#include "camera.h"
#include "color.h"
#include "hittable.h"
#include "hittable_list.h"
#include "interval.h"
#include "ray.h"
#include "sphere.h"
#include "utility.h"
#include "vec.h"
#include <fpng.h>

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

  camera cam(1280, 720);

  cam.render(world);
}