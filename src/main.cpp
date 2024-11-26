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

  world.add(make_shared<Sphere>(Point3{0, 0, -1}, 0.5));
  world.add(make_shared<Sphere>(Point3{0, -100.5, -1}, 100));

  camera cam(1280, 720);

  cam.render(world);
}