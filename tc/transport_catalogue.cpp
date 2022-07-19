#include "transport_catalogue.h"

#include <iomanip>

namespace transport {
void TransportCatalogue::AddRoute(std::string name,
                                  const std::vector<std::string_view>& data,
                                  bool is_round,
                                  std::string_view end_stop) {
  std::string& ref_to_name = buses_names_.emplace_back(std::move(name));
  std::string_view sv_name {ref_to_name};
  std::vector<std::string_view> v_stops;
  for (std::string_view stop : data) {
    v_stops.emplace_back(stops_[stop].name);
    stops_[stop].buses.insert( sv_name);
  }
  routes_[sv_name] = {sv_name, v_stops, is_round, stops_.at(end_stop).name};
}

const Bus* TransportCatalogue::SearchRoute(std::string_view name) const {
  return (!routes_.count(name)) ? nullptr : &routes_.at(name);
}

RouteInfo TransportCatalogue::GetRoute(std::string_view name) {
  RouteInfo result;
  result.name = name.substr(0, name.size());
  if (!routes_.count(name)) {
    result.name.insert(0, "!");
    return result;
  } else {
    const Bus tmp = *SearchRoute(name);
    result.real_stops_count = tmp.stops.size();
    std::set<std::string_view> unique_stops { tmp.stops.begin(),  tmp.stops.end()};
    result.unique_stops_count = unique_stops.size();
    double road_distance = 0;
    double geo_distance = 0;
    for (auto it = tmp.stops.begin(); it < prev(tmp.stops.end()); ++it) {
      road_distance += GetDistBtwStops((*it), *next(it));
      geo_distance += ComputeDistance(SearchStop(*it)->geo, SearchStop(*(it + 1))->geo);
    }
    result.route_length = road_distance;
    result.curvature = road_distance / geo_distance;
    return result;
  }
}

void TransportCatalogue::AddStop(std::string name, double lat, double lng) {
  std::string& ref_to_name = stops_names_.emplace_back(std::move(name));
  std::string_view sv_name {ref_to_name};
  geo::Coordinates geo = {lat, lng};
  stops_[sv_name] = {sv_name, geo, {}};
}

const Stop* TransportCatalogue::SearchStop(std::string_view name) const {
  return (!stops_.count(name)) ? nullptr : &stops_.at(name);
}

StopInfo TransportCatalogue::GetStop(std::string_view name) {
  StopInfo result;
  result.name = name.substr(0, name.size());
  if (!stops_.count(name)) {
    result.name.insert(0, "!");
    return result;
  } else {
    const Stop tmp = *SearchStop(name);
    result.buses = stops_[name].buses;
    return result;
  }
}

void TransportCatalogue::SetDistBtwStops(std::string_view name,
                                         std::string_view name_to, const int dist) {
  const Stop stop = *SearchStop(name);
  const Stop stop_to = *SearchStop(name_to);
  dist_btw_stops_[std::pair(stop.name, stop_to.name)] = dist;
}

int TransportCatalogue::GetDistBtwStops(std::string_view name,
                                        std::string_view name_to) {
  if (dist_btw_stops_.count(std::pair(name, name_to))) {
    return dist_btw_stops_.at(std::pair(name, name_to));
  } else  {
    return dist_btw_stops_.at(std::pair(name_to, name));
  }
}

const TransportCatalogue::RoutesMap &TransportCatalogue::GetRoutesForRender() const {
  return routes_;
}

const TransportCatalogue::StopsMap &TransportCatalogue::GetStopsForRender() const {
  return stops_;
}

const TransportCatalogue::DistMap &TransportCatalogue::GetDistForRouter() const {
  return dist_btw_stops_;
}
}  // namespace transport

