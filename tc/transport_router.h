#pragma once

#include <algorithm>
#include <map>
#include <memory>

#include "domain.h"
#include "geo.h"
#include "router.h"
#include "svg.h"
#include "transport_catalogue.h"

namespace transport {
  struct RouterSettings {
	int bus_wait_time = 6;		/// min
	int bus_velocity_kmh = 40;	/// speed
  };
}  // namespace transport

namespace transport_router {
    using namespace std::literals;
	using namespace transport;

	enum class edge_type {
        WAIT,
        BUS
    };

    struct Edges {
	  edge_type type;
	  std::string_view name;
	  double time;
	  size_t span_count;
    };

    enum class vertex_type {
        IN,
        OUT,
        EMPTY
    };

    struct Vertex {
        std::string_view name;
        vertex_type type = vertex_type::EMPTY;
        size_t id;
    };

	struct StopAsVertexes {
	  Vertex in;
	  Vertex out;
	};

	class TransportRouter {
	public:
	  TransportRouter(const TransportCatalogue& catalogue) : catalogue_(catalogue) {}

	  void SetSettings(const RouterSettings& settings);

	  RouterSettings GetSettings() const;
	  graph::DirectedWeightedGraph<double>& ModifyGraph();
	  const graph::DirectedWeightedGraph<double>& GetGraph() const;

	  void GenerateRouter();
	  void GenerateEmptyRouter();
	  std::unique_ptr<graph::Router<double>>& ModifyRouter();

	  using RouteData = graph::Router<double>::RouteInfo;
	  std::optional<RouteData> GetRoute(std::string_view from, std::string_view to);

	  std::vector<Edges>& ModifyEdgesData();
	  const std::vector<Edges>* GetEdgesData() const;

	  std::map<std::string_view, StopAsVertexes>& ModifyVertexes();
	  const std::map<std::string_view, StopAsVertexes>* GetVertexes() const;
	  const graph::Router<double>::RoutesInternalData& GetRouterData() const;

	private:
	  RouterSettings settings_;
	  const TransportCatalogue& catalogue_;

	  std::unique_ptr<graph::Router<double>> router_ = nullptr;

	  graph::DirectedWeightedGraph<double> graph_;

	  std::map<std::string_view, StopAsVertexes> vertexes_;
	  std::vector<Edges> edges_;

	  void AddStops();
	  double CalculateWeight(int distance);
	  void AddEdges();
	};
} // namespace map_renderer
