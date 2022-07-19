#include "geo.h"

bool geo::Coordinates::operator==(const Coordinates& other) const {
  return lat == other.lat && lng == other.lng;
}

bool geo::Coordinates::operator!=(const Coordinates& other) const {
  return !(*this == other);
}

double geo::ComputeDistance(Coordinates from, Coordinates to) {
  using namespace std;
  static constexpr double dr = PI / GRAD;
  return acos(sin(from.lat * dr) * sin(to.lat * dr)
			  + cos(from.lat * dr) * cos(to.lat * dr)
					* cos(std::abs(from.lng - to.lng) * dr))
		 * R_EARTH;
}
