#pragma once

#include <cmath>
#include <functional>
#include <iomanip>
#include <iostream>

namespace geo {

  static constexpr double PI = 3.1415926535;
  static constexpr double GRAD = 180.;
  static constexpr int R_EARTH = 6371000;

  struct Coordinates {
	double lat = .0;  //широта
	double lng = .0;  //долгота
	bool is_empty = true;
	bool operator==(const Coordinates& other) const;
	bool operator!=(const Coordinates& other) const;
  };

  double ComputeDistance(Coordinates from, Coordinates to);

}  // namespace geo
