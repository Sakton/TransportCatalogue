#pragma once

#include "domain.h"

#include <unordered_set>
#include <unordered_map>
#include <vector>
#include <string>
#include <string_view>
#include <deque>
#include <cassert>
#include <iostream>
#include <set>

namespace transport {
struct TwoStopNameHasher {
  size_t operator()(
      const std::pair<std::string_view, std::string_view>& stop_to_stop) const {
    return sv_hasher(stop_to_stop.first) + 77 * sv_hasher(stop_to_stop.second);
  }
  std::hash<std::string_view> sv_hasher;
};

class TransportCatalogue {
 public:
  ///Types
  using DistMap = std::unordered_map<std::pair<std::string_view, std::string_view>, int,
                                     TwoStopNameHasher>;
  using RoutesMap = std::unordered_map<std::string_view, Bus>;
  using StopsMap = std::unordered_map<std::string_view, Stop>;
  ///Catalogue
  TransportCatalogue() = default;
  void AddRoute(std::string name, const std::vector<std::string_view>& data,
                bool is_round, std::string_view end_stop);
  const Bus* SearchRoute(std::string_view name) const;
  RouteInfo GetRoute(std::string_view name);
  void AddStop(std::string name, double lat, double lng);
  const Stop* SearchStop(std::string_view name) const;
  StopInfo GetStop(std::string_view name);
  void SetDistBtwStops(std::string_view name, std::string_view name_to, const int dist);
  int GetDistBtwStops(std::string_view name, std::string_view name_to);
  /// For map
  const RoutesMap &GetRoutesForRender() const;
  const StopsMap& GetStopsForRender() const;
  const DistMap& GetDistForRouter() const;

 private:
  std::deque<std::string> stops_names_;
  std::deque<std::string> buses_names_;
  DistMap dist_btw_stops_;
  std::unordered_map<std::string_view, Stop> stops_;
  std::unordered_map<std::string_view, Bus> routes_;
};
}  // namespace transport
