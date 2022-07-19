#include "serialization.h"

#include <filesystem>
#include <fstream>
#include <variant>
#include "svg.h"
#include <iostream>

namespace serial {
  void Serializator::SetSettings(const transport::SerializationSettings& settings) {
	settings_ = settings;
  }
  void Serializator::Serialize() {
	proto_transport::TransportCatalogue base;

	*base.mutable_catalogue() = std::move(SerializeCatalogueData());
	*base.mutable_map_renderer() = std::move(SerializeMapRendererData());
	*base.mutable_transport_router() = std::move(SerializeTransportRouterData());

	std::filesystem::path path = settings_.file_name;
	std::ofstream out_file(path, std::ios::binary);
	base.SerializeToOstream(&out_file);
  }
  /// CATALOG
  proto_transport::Catalogue Serializator::SerializeCatalogueData() {
	proto_transport::Catalogue tmp_catalogue;
	int s = 0;
	for (auto& [stop_name, stop_data] : catalogue_.GetStopsForRender()) {
	  ser_stops_ind[stop_name] = s;
	  tmp_catalogue.add_stops();
	  *tmp_catalogue.mutable_stops(s) = std::move(SerializeStopData(stop_data));
	  ++s;
	}
	int b = 0;
	for (auto& [bus_name, bus_data] : catalogue_.GetRoutesForRender()) {
	  ser_buses_ind[bus_name] = b;
	  tmp_catalogue.add_buses();
	  *tmp_catalogue.mutable_buses(b)
		  = std::move(SerializeBusData(bus_data, ser_stops_ind));
	  ++b;
	}
	int d = 0;
	for (auto& [stops, length] : catalogue_.GetDistForRouter()) {
	  tmp_catalogue.add_distances();
	  *tmp_catalogue.mutable_distances(d)
		  = std::move(SerializeDistanceData(stops, length, ser_stops_ind));
	  ++d;
	}
	return tmp_catalogue;
  }

  proto_transport::Stop Serializator::SerializeStopData(
	  const transport::Stop& stop_data) {
	proto_transport::Stop tmp_stop;
	std::string s_name {stop_data.name};
	tmp_stop.set_name(s_name);
	tmp_stop.mutable_coords()->set_geo_lat(stop_data.geo.lat);
	tmp_stop.mutable_coords()->set_geo_lng(stop_data.geo.lng);
	return tmp_stop;
  }

  proto_transport::Bus Serializator::SerializeBusData(
	  const transport::Bus& bus_data, const std::map<std::string_view, int>& stops_ind) {
	proto_transport::Bus tmp_bus;
	std::string s_name {bus_data.name};
	tmp_bus.set_name(s_name);
	for (std::string_view stop : bus_data.stops) {
	  tmp_bus.add_stop_index(stops_ind.at(stop));
	}
	tmp_bus.set_round_trip(bus_data.is_round_trip);
	tmp_bus.set_end_stop_ind(stops_ind.at(bus_data.end_stop));
	return tmp_bus;
  }

  proto_transport::Dist Serializator::SerializeDistanceData(
	  const std::pair<std::string_view, std::string_view> stops, int length,
	  const std::map<std::string_view, int>& stops_ind) {
	proto_transport::Dist tmp_dist;
	tmp_dist.set_from(stops_ind.at(stops.first));
	tmp_dist.set_to(stops_ind.at(stops.second));
	tmp_dist.set_distance(length);
	return tmp_dist;
  }
  /// MAP
  proto_transport::MapRenderer Serializator::SerializeMapRendererData() {
	proto_transport::MapRenderer tmp_map_renderer;

	*tmp_map_renderer.mutable_settings() = std::move(SerializeRenderSettingsData());

	return tmp_map_renderer;
  }

  proto_transport::RenderSettings Serializator::SerializeRenderSettingsData() {
	proto_transport::RenderSettings tmp_render_settings;
	transport::RenderSettings cat_rend_set = renderer_.GetRenderSettings();
	tmp_render_settings.set_width(cat_rend_set.width);
	tmp_render_settings.set_height(cat_rend_set.height);
	tmp_render_settings.set_padding(cat_rend_set.padding);
	tmp_render_settings.set_line_width(cat_rend_set.line_width);
	tmp_render_settings.set_stop_radius(cat_rend_set.stop_radius);
	tmp_render_settings.set_bus_label_font_size(cat_rend_set.bus_label_font_size);
	tmp_render_settings.add_bus_label_offset(cat_rend_set.bus_label_offset[0]);
	tmp_render_settings.add_bus_label_offset(cat_rend_set.bus_label_offset[1]);
	tmp_render_settings.set_stop_label_font_size(cat_rend_set.stop_label_font_size);
	tmp_render_settings.add_stop_label_offset(cat_rend_set.stop_label_offset[0]);
	tmp_render_settings.add_stop_label_offset(cat_rend_set.stop_label_offset[1]);
	*tmp_render_settings.mutable_underlayer_color()
		= SerializeColorData(cat_rend_set.underlayer_color);
	tmp_render_settings.set_underlayer_width(cat_rend_set.underlayer_width);

	for (int i = 0; i < cat_rend_set.color_palette.size(); ++i) {
	  tmp_render_settings.add_color_palette();
	  *tmp_render_settings.mutable_color_palette(i)
		  = SerializeColorData(cat_rend_set.color_palette[i]);
	}

	return tmp_render_settings;
  }

  proto_transport::Color Serializator::SerializeColorData(
	  const svg::Color& catalogue_color) {
	proto_transport::Color tmp_color;
	if (std::holds_alternative<std::monostate>(catalogue_color)) {
	  return {};
	} else if (std::holds_alternative<std::string>(catalogue_color)) {
	  tmp_color.set_string_name(std::get<std::string>(catalogue_color));
	} else if (std::holds_alternative<svg::Rgb>(catalogue_color)) {
	  svg::Rgb tmp_rgb_color = std::get<svg::Rgb>(catalogue_color);
	  tmp_color.mutable_rgb()->set_r(tmp_rgb_color.red);
	  tmp_color.mutable_rgb()->set_g(tmp_rgb_color.green);
	  tmp_color.mutable_rgb()->set_b(tmp_rgb_color.blue);
	} else {
	  svg::Rgba tmp_rgba_color = std::get<svg::Rgba>(catalogue_color);
	  tmp_color.mutable_rgba()->set_r(tmp_rgba_color.red);
	  tmp_color.mutable_rgba()->set_g(tmp_rgba_color.green);
	  tmp_color.mutable_rgba()->set_b(tmp_rgba_color.blue);
	  tmp_color.mutable_rgba()->set_opacity(tmp_rgba_color.opacity);
	}
	return tmp_color;
  }

  /// ROUTER
  proto_transport::TransportRouter Serializator::SerializeTransportRouterData() {
	proto_transport::TransportRouter tmp_transp_router;

	*tmp_transp_router.mutable_settings() = std::move(SerializeRouterSettingsData());
	*tmp_transp_router.mutable_transport_router()
		= std::move(SerializeTransportRouterClassData());
	*tmp_transp_router.mutable_router() = std::move(SerializeRouterData());
	*tmp_transp_router.mutable_graph() = std::move(SerializeGraphData());

	return tmp_transp_router;
  }

  proto_transport::RouterSettings Serializator::SerializeRouterSettingsData() {
	proto_transport::RouterSettings tmp_router_settings;
	transport::RouterSettings cat_router_set = router_.GetSettings();
	tmp_router_settings.set_bus_velocity_kmh(cat_router_set.bus_velocity_kmh);
	tmp_router_settings.set_bus_wait_time(cat_router_set.bus_wait_time);
	return tmp_router_settings;
  }

  proto_transport::TransportRouterData Serializator::SerializeTransportRouterClassData() {
	proto_transport::TransportRouterData tmp_transp_router_class_data;
	int i = 0;
	for (const auto& [name, data] : *router_.GetVertexes()) {
	  tmp_transp_router_class_data.add_vertexes();
	  tmp_transp_router_class_data.mutable_vertexes(i)->set_stop_id(
		  ser_stops_ind.at(name));
	  tmp_transp_router_class_data.mutable_vertexes(i)->set_id(data.in.id);
	  ++i;
	}
	int j = 0;
	for (const auto& edge : *router_.GetEdgesData()) {
	  tmp_transp_router_class_data.add_edges();
	  tmp_transp_router_class_data.mutable_edges(j)->set_type(
		  static_cast<int>(edge.type));
	  if (edge.type == transport_router::edge_type::WAIT) {
		tmp_transp_router_class_data.mutable_edges(j)->set_name_id(
			ser_stops_ind.at(edge.name));
	  } else {
		tmp_transp_router_class_data.mutable_edges(j)->set_name_id(
			ser_buses_ind.at(edge.name));
	  }
	  tmp_transp_router_class_data.mutable_edges(j)->set_span_count(edge.span_count);
	  tmp_transp_router_class_data.mutable_edges(j)->set_time(edge.time);
	  ++j;
	}
	return tmp_transp_router_class_data;
  }

  proto_transport::Router Serializator::SerializeRouterData() {
	proto_transport::Router tmp_router;
	int i = 0;
	for (const auto& data_vector : router_.GetRouterData()) {
	  int j = 0;
	  tmp_router.add_routes_internal_data();
	  for (const auto& data : data_vector) {
		tmp_router.mutable_routes_internal_data(i)->add_route_internal_data_elem();
		if (data) {
		  proto_transport::RouteInternalData& elem_data
			  = *tmp_router.mutable_routes_internal_data(i)
					 ->mutable_route_internal_data_elem(j)
					 ->mutable_data();
		  elem_data.set_weight(data.value().weight);
		  if (data.value().prev_edge) {
			elem_data.set_edgeid(data.value().prev_edge.value());
		  }
		}
		++j;
	  }
	  ++i;
	}
	return tmp_router;
  }

  proto_transport::Graph Serializator::SerializeGraphData() {
	proto_transport::Graph tmp_graph;
	for (int i = 0; i < router_.GetGraph().GetEdgeCount(); ++i) {
	  graph::Edge tmp_cat_edge = router_.GetGraph().GetEdge(i);
	  tmp_graph.add_edges();
	  proto_transport::Edge& tmp_base_edge = *tmp_graph.mutable_edges(i);
	  tmp_base_edge.set_from(tmp_cat_edge.from);
	  tmp_base_edge.set_to(tmp_cat_edge.to);
	  tmp_base_edge.set_weight(tmp_cat_edge.weight);
	}
	for (size_t i = 0; i < router_.GetGraph().GetVertexCount(); ++i) {
	  tmp_graph.add_incidence_lists();
	  proto_transport::IncidenceList& tmp_base_inc_list
		  = *tmp_graph.mutable_incidence_lists(i);
	  int j = 0;
	  for (const auto inc_edge : router_.GetGraph().GetIncidentEdges(i)) {
		tmp_base_inc_list.add_edges(inc_edge);
		++j;
	  }
	}
	return tmp_graph;
  }

}  // namespace serial

///////--------------------------------------------------------------------------/////////

namespace deserial {

  /// DESERIALIZATION
  void DeSerializator::DeSerialize() {
	std::filesystem::path path = settings_.file_name;
	std::ifstream in_file(path, std::ios::binary);

	proto_transport::TransportCatalogue base;
	base.ParseFromIstream(&in_file);

	DeserializeCatalogueData(base.catalogue());
	DeserializeMapRendererData(base.map_renderer());
	router_.GenerateEmptyRouter();
	DeserializeTransportRouterData(base.transport_router());
	DeserializeRouterData(base.transport_router().router());
	DeserializeGraphData(base.transport_router().graph());
  }

  void DeSerializator::SetSettings(const transport::SerializationSettings& settings) {
	settings_ = settings;
  }
  /// CATALOGUE
  void DeSerializator::DeserializeCatalogueData(const proto_transport::Catalogue& base) {
	for (int i = 0; i < base.stops_size(); ++i) {
	  catalogue_.AddStop(base.stops(i).name(), base.stops(i).coords().geo_lat(),
						 base.stops(i).coords().geo_lng());
	  deser_stops_ind[i] = catalogue_.SearchStop(base.stops(i).name())->name;
	}
	for (int i = 0; i < base.buses_size(); ++i) {
	  std::vector<std::string_view> stops;
	  for (int j = 0; j < base.buses(i).stop_index_size(); ++j) {
		stops.push_back(deser_stops_ind.at(base.buses(i).stop_index(j)));
	  }
	  catalogue_.AddRoute(base.buses(i).name(), stops, base.buses(i).round_trip(),
						  deser_stops_ind.at(base.buses(i).end_stop_ind()));
	  deser_buses_ind[i] = catalogue_.SearchRoute(base.buses(i).name())->name;
	}
	for (int i = 0; i < base.distances_size(); ++i) {
	  catalogue_.SetDistBtwStops(deser_stops_ind.at(base.distances(i).from()),
								 deser_stops_ind.at(base.distances(i).to()),
								 base.distances(i).distance());
	}
  }

  /// MAP RENDERER
  void DeSerializator::DeserializeMapRendererData(
	  const proto_transport::MapRenderer& base_map_renderer) {
	renderer_.SetSettings(DeserializeRenderSettingsData(base_map_renderer.settings()));
  }

  transport::RenderSettings DeSerializator::DeserializeRenderSettingsData(
	  const proto_transport::RenderSettings& base_render_settings) {
	transport::RenderSettings tmp_settings;
	tmp_settings.width = base_render_settings.width();
	tmp_settings.height = base_render_settings.height();
	tmp_settings.padding = base_render_settings.padding();
	tmp_settings.line_width = base_render_settings.line_width();
	tmp_settings.stop_radius = base_render_settings.stop_radius();
	tmp_settings.bus_label_font_size = base_render_settings.bus_label_font_size();
	tmp_settings.bus_label_offset[0] = base_render_settings.bus_label_offset(0);
	tmp_settings.bus_label_offset[1] = base_render_settings.bus_label_offset(1);
	tmp_settings.stop_label_font_size = base_render_settings.stop_label_font_size();
	tmp_settings.stop_label_offset[0] = base_render_settings.stop_label_offset(0);
	tmp_settings.stop_label_offset[1] = base_render_settings.stop_label_offset(1);
	tmp_settings.underlayer_color
		= DeserializeColorData(base_render_settings.underlayer_color());
	tmp_settings.underlayer_width = base_render_settings.underlayer_width();
	tmp_settings.color_palette.reserve(base_render_settings.color_palette_size());
	for (int i = 0; i < base_render_settings.color_palette_size(); ++i) {
	  tmp_settings.color_palette.emplace_back(
		  std::move(DeserializeColorData(base_render_settings.color_palette(i))));
	}
	return tmp_settings;
  }

  svg::Color DeSerializator::DeserializeColorData(
	  const proto_transport::Color& base_color) {
	svg::Color empty_color {};
	switch (base_color.color_case()) {
	  case proto_transport::Color::ColorCase::COLOR_NOT_SET:
		return empty_color;
		break;
	  case proto_transport::Color::ColorCase::kStringName:
		return base_color.string_name();
		break;
	  case proto_transport::Color::ColorCase::kRgb:
		return svg::Rgb(base_color.rgb().r(), base_color.rgb().g(), base_color.rgb().b());
		break;
	  case proto_transport::Color::ColorCase::kRgba:
		return svg::Rgba(base_color.rgba().r(), base_color.rgba().g(),
						 base_color.rgba().b(), base_color.rgba().opacity());
		break;
	}
	return empty_color;
  }

  /// ROUTER
  void DeSerializator::DeserializeTransportRouterData(
	  const proto_transport::TransportRouter& base_transport_router) {
	router_.SetSettings(
		DeserializeTrasnportRouterSettingsData(base_transport_router.settings()));
	DeserializeTransportRouterClassData(base_transport_router.transport_router());
	DeserializeGraphData(base_transport_router.graph());
  }
  /// ROUTER CLASS
  transport_router::RouterSettings DeSerializator::DeserializeTrasnportRouterSettingsData(
	  const proto_transport::RouterSettings& base_router_settings) {
	transport_router::RouterSettings tmp_settings;
	tmp_settings.bus_velocity_kmh = base_router_settings.bus_velocity_kmh();
	tmp_settings.bus_wait_time = base_router_settings.bus_wait_time();
	return tmp_settings;
  }

  void DeSerializator::DeserializeTransportRouterClassData(
	  const proto_transport::TransportRouterData& base_transport_router_data) {
	router_.ModifyVertexes()
		= std::move(DeserializeTranspRouterVertexesData(base_transport_router_data));
	router_.ModifyEdgesData()
		= DeserializeTranspRouterEdgesData(base_transport_router_data);
  }

  DeSerializator::Vertexes DeSerializator::DeserializeTranspRouterVertexesData(
	  const proto_transport::TransportRouterData& base_transport_router_data) {
	Vertexes tmp_vertexes;
	for (int i = 0; i < base_transport_router_data.vertexes_size(); ++i) {
	  tmp_vertexes[deser_stops_ind.at(base_transport_router_data.vertexes(i).stop_id())]
		  .in.id
		  = base_transport_router_data.vertexes(i).id();
	}
	return tmp_vertexes;
  }

  DeSerializator::Edges DeSerializator::DeserializeTranspRouterEdgesData(
	  const proto_transport::TransportRouterData& base_transport_router_data) {
	Edges tmp_edges;
	tmp_edges.reserve(base_transport_router_data.edges_size());
	for (int i = 0; i < base_transport_router_data.edges_size(); ++i) {
	  transport_router::Edges tmp_edge;
	  switch (base_transport_router_data.edges(i).type()) {
		case 0:
		  tmp_edge.type = transport_router::edge_type::WAIT;
		  tmp_edge.name
			  = deser_stops_ind.at(base_transport_router_data.edges(i).name_id());
		  break;
		case 1:
		  tmp_edge.type = transport_router::edge_type::BUS;
		  tmp_edge.name
			  = deser_buses_ind.at(base_transport_router_data.edges(i).name_id());
		  break;
	  }
	  tmp_edge.time = base_transport_router_data.edges(i).time();
	  tmp_edge.span_count = base_transport_router_data.edges(i).span_count();
	  tmp_edges.emplace_back(std::move(tmp_edge));
	}
	return tmp_edges;
  }
  /// ROUTER
  void DeSerializator::DeserializeRouterData(const proto_transport::Router& base_router) {
	std::vector<std::vector<std::optional<graph::Router<double>::RouteInternalData>>>&
		routes_internal_data
		= router_.ModifyRouter().get()->ModifyRoutesInternalData();
	routes_internal_data.resize(base_router.routes_internal_data_size());
	for (int i = 0; i < base_router.routes_internal_data_size(); ++i) {
	  routes_internal_data[i].reserve(
		  base_router.routes_internal_data(i).route_internal_data_elem_size());
	  for (int j = 0;
		   j < base_router.routes_internal_data(i).route_internal_data_elem_size(); ++j) {
		proto_transport::RouteInternalDataVectorElem base_elem
			= base_router.routes_internal_data(i).route_internal_data_elem(j);
		routes_internal_data[i].emplace_back(
			std::move(DeserializeRouteInternalData(base_elem)));
	  }
	}
  }

  std::optional<graph::Router<double>::RouteInternalData>
  DeSerializator::DeserializeRouteInternalData(
	  proto_transport::RouteInternalDataVectorElem& base) {
	graph::Router<double>::RouteInternalData res {};
	switch (base.elem_case()) {
	  case proto_transport::RouteInternalDataVectorElem::ElemCase::ELEM_NOT_SET:
		return std::nullopt;
		break;
	  case proto_transport::RouteInternalDataVectorElem::ElemCase::kData:
		res.weight = base.data().weight();
		switch (base.data().prev_edge_case()) {
		  case proto_transport::RouteInternalData::PrevEdgeCase::kEdgeid:
			res.prev_edge = std::make_optional(base.data().edgeid());
			break;
		  case proto_transport::RouteInternalData::PrevEdgeCase::PREV_EDGE_NOT_SET:
			res.prev_edge = std::nullopt;
			break;
		}
	}
	return res;
  }
  /// GRAPH
  void DeSerializator::DeserializeGraphData(
	  const proto_transport::Graph& base_graph_data) {
	router_.ModifyGraph().ModifyEdges()
		= std::move(DeserializeGraphEdgesData(base_graph_data));
	router_.ModifyGraph().ModifyIncidenceLists()
		= std::move(DeserializeGraphIncidenceListsData(base_graph_data));
  }

  std::vector<graph::Edge<double>> DeSerializator::DeserializeGraphEdgesData(
	  const proto_transport::Graph& base_graph_data) {
	std::vector<graph::Edge<double>> tmp_edges;
	;
	tmp_edges.reserve(base_graph_data.edges_size());
	for (int i = 0; i < base_graph_data.edges_size(); ++i) {
	  graph::Edge<double> tmp_edge;
	  tmp_edge.from = base_graph_data.edges(i).from();
	  tmp_edge.to = base_graph_data.edges(i).to();
	  tmp_edge.weight = base_graph_data.edges(i).weight();
	  tmp_edges.emplace_back(std::move(tmp_edge));
	}
	return tmp_edges;
  }

  std::vector<graph::IncidenceList> DeSerializator::DeserializeGraphIncidenceListsData(
	  const proto_transport::Graph& base_graph_data) {
	std::vector<graph::IncidenceList> tmp_inc_lists;
	tmp_inc_lists.reserve(base_graph_data.incidence_lists_size());
	for (int i = 0; i < base_graph_data.incidence_lists_size(); ++i) {
	  graph::IncidenceList tmp_list;
	  tmp_list.reserve(base_graph_data.incidence_lists(i).edges_size());
	  for (int j = 0; j < base_graph_data.incidence_lists(i).edges_size(); ++j) {
		tmp_list.emplace_back(base_graph_data.incidence_lists(i).edges(j));
	  }
	  tmp_inc_lists.emplace_back(std::move(tmp_list));
	}
	return tmp_inc_lists;
  }
}  // namespace deserial
