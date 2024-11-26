#ifndef INTERVAL_H
#define INTERVAL_H

#include "utility.h"
class Interval {
public:
  double min, max;

  Interval() : min(+infinity), max(-infinity) {} // Default interval is empty

  Interval(double min, double max) : min(min), max(max) {}

  [[nodiscard]] double size() const { return max - min; }

  [[nodiscard]] bool contains(double x) const { return min <= x && x <= max; }

  [[nodiscard]] bool surrounds(double x) const { return min < x && x < max; }

  static const Interval empty, universe;
};

const Interval Interval::empty = Interval(+infinity, -infinity);
const Interval Interval::universe = Interval(-infinity, +infinity);

#endif