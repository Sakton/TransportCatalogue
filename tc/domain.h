#pragma once

#include <set>
#include <string>
#include <string_view>
#include <vector>

#include "geo.h"

namespace transport {
  struct Stop {
	std::string_view name;
	geo::Coordinates geo;
	std::set<std::string_view> buses;
	bool operator==(const Stop& other) const;
  };

  struct Bus {
	std::string_view name;
	std::vector<std::string_view> stops;
	bool is_round_trip = false;
	std::string_view end_stop;
	bool operator==(const Bus& other) const;
  };

  struct RouteInfo {
	std::string name;
	size_t real_stops_count;
	size_t unique_stops_count;
	double route_length;
	double curvature;
  };

  struct StopInfo {
	std::string name;
	std::set<std::string_view> buses;
  };
}  // namespace transport
