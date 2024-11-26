#ifndef UTLITY_H
#define UTLITY_H

#include <cmath>
#include <iostream>
#include <limits>
#include <memory>
#include <numbers>

// C++ Std Usings

using std::make_shared;
using std::shared_ptr;

// Constants

const double infinity = std::numeric_limits<double>::infinity();
const double pi = std::numbers::pi;

// Utility Functions

inline double degrees_to_radians(double degrees) {
  return degrees * pi / 180.0;
}

#endif