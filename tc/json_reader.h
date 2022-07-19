#pragma once

#include <iomanip>
#include <iostream>
#include <sstream>
#include <variant>

#include "json.h"
#include "json_builder.h"
#include "map_renderer.h"
#include "request_handler.h"
#include "serialization.h"
#include "transport_catalogue.h"
#include "transport_router.h"

namespace jsoninputer {
using namespace std::literals;
using namespace transport;
using namespace json;
using namespace map_renderer;
using namespace transport_router;

namespace detail {
enum class QueryType { BASE = 0, STAT, RENDER, EMPTY };
enum class TypeData { BUS, STOP, MAP, ROUTE, EMPTY };

struct PreparedData {
  QueryType query_type = QueryType::EMPTY;
  TypeData type_data = TypeData::EMPTY;
  virtual ~PreparedData() = default;
};

struct PreparedStatRoute {
  std::string from;
  std::string to;
};

struct PreparedStat : public PreparedData {
  std::string name;
  int id = 0;
  PreparedStatRoute route;
};

struct PreparedStop : public PreparedData {
  std::string name;
  double latitude = 0;
  double longitude = 0;
  std::map<std::string, int> road_distances;
};

struct PreparedBus : public PreparedData {
  std::string name;
  bool is_roundtrip = false;
  std::vector<std::string> stops;
};

std::map<std::string, int> DistStops(const Dict& dic);
PreparedStop BaseStop(const Dict& dic);
std::vector<std::string> StopsBus(const Array& arr);
PreparedBus BaseBus(const Dict& dic);

PreparedStat Stat(const Dict& dic);

svg::Color ColorNode(const Node& node);
transport::RenderSettings RenderMap(const Dict& dic);
}	 // namespace detail

using namespace detail;

class JsonReader {
public:
  using StopDist = std::vector<std::pair<std::string, std::map<std::string, int>>>;

  JsonReader(transport::TransportCatalogue& catalogue, MapRenderer& renderer,
			 transport_router::TransportRouter& router,
			 std::variant<serial::Serializator, deserial::DeSerializator>& serialization)
	  : catalogue_(catalogue),
		renderer_(renderer),
		router_(router),
		serialization_(serialization) {}

  void ReadInput(std::istream& input);
  void AddCatalogue();
  void PrintRequests(std::ostream& out, RequestHandler& request_handler);

 private:
  void InitDoc(std::istream& in);
  void AddBase(const std::vector<Node>& vec);
  void AddStops(StopDist& stops_w_dist);
  void AddBusss();
  void AddStat(const std::vector<Node>& vec);
  void AddRender(const std::map<std::string, json::Node>& dic);
  void AddRouting(const std::map<std::string, Node>& dic);
  void AddSerialization(const std::map<std::string, Node>& dic);
  void StopStatPrepare(const transport::StopInfo& request, Builder& dict) const;
  void BusStatPrepare(const transport::RouteInfo& request, Builder& dict) const;
  ///PrintsData
  void PrintStop(ostream& out, PreparedStat* s) const;
  void PrintBus(ostream& out, PreparedStat* s) const;
  void PrintMap(ostream& out, PreparedStat* s, RequestHandler& request_handler) const;
  void PrintRoute(ostream& out, PreparedStat* s) const;

private:
  transport::TransportCatalogue& catalogue_;
  std::optional<Document> document_opt_;
  std::vector<std::unique_ptr<PreparedData>> requests_;
  MapRenderer& renderer_;
  transport_router::TransportRouter& router_;
  std::variant<serial::Serializator, deserial::DeSerializator>& serialization_;
};
}  // namespace json_reader
