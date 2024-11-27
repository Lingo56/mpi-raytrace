#ifndef COLOR_H
#define COLOR_H

#include "interval.h"
#include "vec.h"

#include <iostream>

using Color = Vec3;

inline void write_color(std::ostream &out, const Color &pixel_color) {
  // auto r = pixel_color.x();
  // auto g = pixel_color.y();
  // auto b = pixel_color.z();

  // Translate the [0,1] component values to the byte range [0,255].
  // int rbyte = int(255.999 * r);
  // int gbyte = int(255.999 * g);
  // int bbyte = int(255.999 * b);

  static const Interval intensity(0.000, 0.999);
  auto rgb = Color{
      256 * intensity.clamp(pixel_color[0]),
      256 * intensity.clamp(pixel_color[1]),
      256 * intensity.clamp(pixel_color[2])
  };

  // Write out the pixel color components.
  out << rgb[0] << ' ' << rgb[1] << ' ' << rgb[2] << '\n';
}

#endif