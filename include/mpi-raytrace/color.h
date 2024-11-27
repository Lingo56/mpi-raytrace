#ifndef COLOR_H
#define COLOR_H

#include "interval.h"
#include "vec.h"

#include <cmath>
#include <iostream>

using Color = Vec3;

inline double linear_to_gamma(double linear_component) {
  if (linear_component > 0)
    return std::sqrt(linear_component);

  return 0;
}

inline void write_color(std::ostream &out, const Color &pixel_color) {
  static const Interval intensity(0.000, 0.999);
  auto rgb = Color{
      256 * intensity.clamp(linear_to_gamma(pixel_color[0])),
      256 * intensity.clamp(linear_to_gamma(pixel_color[1])),
      256 * intensity.clamp(linear_to_gamma(pixel_color[2]))
  };

  // Write out the pixel color components.
  out << rgb[0] << ' ' << rgb[1] << ' ' << rgb[2] << '\n';
}

#endif