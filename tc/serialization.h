#pragma once

#include <string>
#include <transport_catalogue.pb.h> 
 
#include "transport_catalogue.h"
#include "map_renderer.h"
#include "transport_router.h"

using namespace std::literals;

namespace transport {
  struct SerializationSettings {
	std::string file_name = ""s;
  };
}  // namespace transport

namespace serial {
  class Serializator {
  public:
	Serializator(transport::TransportCatalogue& catalogue,
				 map_renderer::MapRenderer& renderer,
				 transport_router::TransportRouter& router)
		: catalogue_(catalogue), renderer_(renderer), router_(router) {}

	void SetSettings(const transport::SerializationSettings& settings);
	void Serialize();

  private:
	/// Catalogue
	proto_transport::Catalogue SerializeCatalogueData();
	proto_transport::Stop SerializeStopData(const transport::Stop& stop_data);
	proto_transport::Bus SerializeBusData(
		const transport::Bus& bus_data, const std::map<std::string_view, int>& stops_ind);
	proto_transport::Dist SerializeDistanceData(
		const std::pair<std::string_view, std::string_view> stops, int length,
		const std::map<std::string_view, int>& stops_ind);
	/// MapRenderer
	proto_transport::MapRenderer SerializeMapRendererData();
	proto_transport::RenderSettings SerializeRenderSettingsData();
	proto_transport::Color SerializeColorData(const svg::Color& catalogue_color);
	/// TransportRouter
	proto_transport::TransportRouter SerializeTransportRouterData();
	proto_transport::RouterSettings SerializeRouterSettingsData();
	proto_transport::TransportRouterData SerializeTransportRouterClassData();
	proto_transport::Router SerializeRouterData();
	proto_transport::Graph SerializeGraphData();

  private:
	transport::SerializationSettings settings_;
	transport::TransportCatalogue& catalogue_;
	map_renderer::MapRenderer& renderer_;
	transport_router::TransportRouter& router_;

	std::map<std::string_view, int> ser_stops_ind;
	std::map<std::string_view, int> ser_buses_ind;
  };

}  // namespace serial

namespace deserial {
  class DeSerializator {
  public:
	DeSerializator(transport::TransportCatalogue& catalogue,
				   map_renderer::MapRenderer& renderer,
				   transport_router::TransportRouter& router)
		: catalogue_(catalogue), renderer_(renderer), router_(router) {}

	void DeSerialize();
	void SetSettings(const transport::SerializationSettings& settings);

  private:
	/// Catalogue
	void DeserializeCatalogueData(const proto_transport::Catalogue& base);
	/// MapRenderer
	void DeserializeMapRendererData(
		const proto_transport::MapRenderer& base_map_renderer);
	transport::RenderSettings DeserializeRenderSettingsData(
		const proto_transport::RenderSettings& base_render_settings);
	svg::Color DeserializeColorData(const proto_transport::Color& base_color);
	/// TransportRouter
	void DeserializeTransportRouterData(
		const proto_transport::TransportRouter& base_transport_router);
	transport_router::RouterSettings DeserializeTrasnportRouterSettingsData(
		const proto_transport::RouterSettings& base_router_settings);
	/// Transport router class
	void DeserializeTransportRouterClassData(
		const proto_transport::TransportRouterData& base);
	using Vertexes = std::map<std::string_view, transport_router::StopAsVertexes>;
	Vertexes DeserializeTranspRouterVertexesData(
		const proto_transport::TransportRouterData& base_transport_router_data);
	using Edges = std::vector<transport_router::Edges>;
	Edges DeserializeTranspRouterEdgesData(
		const proto_transport::TransportRouterData& base_transport_router_data);
	/// Graph
	void DeserializeGraphData(const proto_transport::Graph& base_graph_data);
	std::vector<graph::Edge<double>> DeserializeGraphEdgesData(
		const proto_transport::Graph& base_graph_data);
	std::vector<graph::IncidenceList> DeserializeGraphIncidenceListsData(
		const proto_transport::Graph& base_graph_data);
	/// Router
	void DeserializeRouterData(const proto_transport::Router& base_router);
	std::optional<graph::Router<double>::RouteInternalData> DeserializeRouteInternalData(
		proto_transport::RouteInternalDataVectorElem& base);

  private:
	transport::SerializationSettings settings_;
	transport::TransportCatalogue& catalogue_;
	map_renderer::MapRenderer& renderer_;
	transport_router::TransportRouter& router_;

	std::map<int, std::string_view> deser_stops_ind;
	std::map<int, std::string_view> deser_buses_ind;
  };

}  // namespace deserial
