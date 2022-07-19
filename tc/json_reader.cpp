#include "json_reader.h"

namespace jsoninputer {
  using namespace std::literals;
  using namespace transport;

  namespace detail {
	std::map<std::string, int> DistStops(const json::Dict& dic) {
	  std::map<std::string, int> res;
	  for (const auto& elem : dic) {
		res[elem.first] = elem.second.AsInt();
	  }
	  return res;
	}

	PreparedStop BaseStop(const json::Dict& dic) {
	  PreparedStop stop;
	  stop.query_type = QueryType::BASE;
	  stop.type_data = TypeData::STOP;
	  stop.name = dic.at("name"s).AsString();
	  stop.latitude = dic.at("latitude"s).AsDouble();
	  stop.longitude = dic.at("longitude"s).AsDouble();
	  if (dic.count("road_distances"s)) {
		stop.road_distances = std::move(DistStops(dic.at("road_distances"s).AsDict()));
	  }
	  return stop;
	}

	std::vector<std::string> StopsBus(const json::Array& arr) {
	  std::vector<std::string> res;
	  for (const auto& elem : arr) {
		res.emplace_back(elem.AsString());
	  }
	  return res;
	}

	PreparedBus BaseBus(const json::Dict& dic) {
	  PreparedBus bus;
	  bus.query_type = QueryType::BASE;
	  bus.type_data = TypeData::BUS;
	  bus.name = dic.at("name"s).AsString();
	  bus.stops = std::move(StopsBus(dic.at("stops"s).AsArray()));
	  bus.is_roundtrip = dic.at("is_roundtrip"s).AsBool();
	  return bus;
	}

	PreparedStat Stat(const json::Dict& dic) {
	  PreparedStat stat;
	  stat.query_type = QueryType::STAT;
	  stat.id = dic.at("id"s).AsInt();
	  if (dic.at("type"s).AsString() == "Bus"s) {
		stat.type_data = TypeData::BUS;
		stat.name = dic.at("name"s).AsString();
	  } else if (dic.at("type"s).AsString() == "Stop"s) {
		stat.type_data = TypeData::STOP;
		stat.name = dic.at("name"s).AsString();
	  } else if (dic.at("type"s).AsString() == "Map"s) {
		stat.type_data = TypeData::MAP;
	  } else if (dic.at("type"s).AsString() == "Route"s) {
		stat.type_data = TypeData::ROUTE;
		stat.route.from = dic.at("from"s).AsString();
		stat.route.to = dic.at("to"s).AsString();
	  }
	  return stat;
	}

	svg::Color ColorNode(const json::Node& node) {
	  if (node.IsArray()) {
		if (node.AsArray().size() == 3) {
		  svg::Rgb rgb;
		  rgb.red = node.AsArray()[0].AsInt();
		  rgb.green = node.AsArray()[1].AsInt();
		  rgb.blue = node.AsArray()[2].AsInt();
		  return rgb;
		} else {
		  svg::Rgba rgba;
		  rgba.red = node.AsArray()[0].AsInt();
		  rgba.green = node.AsArray()[1].AsInt();
		  rgba.blue = node.AsArray()[2].AsInt();
		  rgba.opacity = node.AsArray()[3].AsDouble();
		  return rgba;
		}
	  } else {
		return node.AsString();
	  }
	}

	transport::RenderSettings RenderMap(const json::Dict& dic) {
	  using namespace detail;
	  transport::RenderSettings res;
	  res.width = dic.at("width"s).AsDouble();
	  res.height = dic.at("height"s).AsDouble();
	  res.padding = dic.at("padding"s).AsDouble();
	  res.line_width = dic.at("line_width"s).AsDouble();
	  res.stop_radius = dic.at("stop_radius"s).AsDouble();
	  res.bus_label_font_size = dic.at("bus_label_font_size"s).AsInt();
	  res.bus_label_offset[0] = dic.at("bus_label_offset"s).AsArray()[0].AsDouble();
	  res.bus_label_offset[1] = dic.at("bus_label_offset"s).AsArray()[1].AsDouble();
	  res.stop_label_font_size = dic.at("stop_label_font_size"s).AsInt();
	  res.stop_label_offset[0] = dic.at("stop_label_offset"s).AsArray()[0].AsDouble();
	  res.stop_label_offset[1] = dic.at("stop_label_offset"s).AsArray()[1].AsDouble();
	  res.underlayer_color = ColorNode(dic.at("underlayer_color"s));
	  res.underlayer_width = dic.at("underlayer_width"s).AsDouble();
	  for (const auto& node : dic.at("color_palette"s).AsArray()) {
		res.color_palette.push_back(ColorNode(node));
	  }
	  return res;
	}

	transport::RouterSettings RouterMap(const json::Dict& dic) {
	  using namespace detail;
	  transport::RouterSettings res;
	  res.bus_velocity_kmh = dic.at("bus_velocity"s).AsInt();
	  res.bus_wait_time = dic.at("bus_wait_time"s).AsInt();
	  return res;
	}

	SerializationSettings SerializationCatalogue(const json::Dict& dic) {
	  SerializationSettings res;
	  res.file_name = dic.at("file"s).AsString();
	  return res;
	}
  }	 // namespace detail

  using namespace detail;

  void JsonReader::ReadInput(std::istream& input) {
	InitDoc(input);
	for (const auto& elem : document_opt_.value().GetRoot().AsDict()) {
	  if (elem.first == "base_requests"s) {
		AddBase(elem.second.AsArray());
	  } else if (elem.first == "stat_requests"s) {
		AddStat(elem.second.AsArray());
	  } else if (elem.first == "render_settings"s) {
		AddRender(elem.second.AsDict());
	  } else if (elem.first == "routing_settings"s) {
		AddRouting(elem.second.AsDict());
	  } else if (elem.first == "serialization_settings"s) {
		AddSerialization(elem.second.AsDict());
	  }
	}
  }

  void JsonReader::AddStops(StopDist& stops_w_dist) {
	std::for_each(
		requests_.begin(), requests_.end(),
		[this, &stops_w_dist](const std::unique_ptr<PreparedData>& elem) {
		  if (PreparedStop* s = dynamic_cast<PreparedStop*>(elem.get())) {
			catalogue_.AddStop(s->name, s->latitude, s->longitude);
			if (!s->road_distances.empty()) {
			  stops_w_dist.emplace_back(std::make_pair(s->name, s->road_distances));
			}
		  }
		});
  }

  void JsonReader::AddBusss() {
	std::for_each(requests_.begin(), requests_.end(),
				  [this](const std::unique_ptr<PreparedData>& elem) {
					if (PreparedBus* b = dynamic_cast<PreparedBus*>(elem.get())) {
					  std::vector<std::string_view> stops;
					  stops.reserve(b->stops.size() * 2);
					  std::copy(b->stops.begin(), b->stops.end(),
								std::back_inserter(stops));
					  std::string_view end_stop = stops.back();
					  if (!b->is_roundtrip) {
						stops.insert(stops.end(), next(stops.rbegin()), stops.rend());
					  }
					  stops.shrink_to_fit();
					  catalogue_.AddRoute(b->name, stops, b->is_roundtrip, end_stop);
					}
				  });
  }

  void JsonReader::AddCatalogue() {
	StopDist stops_w_dist;
	AddStops(stops_w_dist);
	AddBusss();
	for (const auto& [main_stop, des_stops] : stops_w_dist) {
	  for (const auto& [des_stop, dist] : des_stops) {
		catalogue_.SetDistBtwStops(main_stop, des_stop, dist);
	  }
	}
  }

  void JsonReader::PrintStop(std::ostream& out, PreparedStat* s) const {
	Builder request {};
	request.StartDict().Key("request_id"s).Value(s->id);
	StopStatPrepare(catalogue_.GetStop(s->name), request);
	request.EndDict();
	Print(Document {request.Build()}, out);
  }

  void JsonReader::PrintBus(ostream& out, PreparedStat* s) const {
	Builder request {};
	request.StartDict().Key("request_id"s).Value(s->id);
	BusStatPrepare(catalogue_.GetRoute(s->name), request);
	request.EndDict();
	Print(Document {request.Build()}, out);
  }

  void JsonReader::PrintMap(ostream& out, PreparedStat* s,
							RequestHandler& request_handler) const {
	Builder request {};
	request.StartDict().Key("request_id"s).Value(s->id);
	request_handler.SetCatalogueDataToRender();
	std::stringstream strm;
	renderer_.Render(strm);
	request.Key("map"s).Value(strm.str());
	request.EndDict();
	Print(Document {request.Build()}, out);
  }

  void JsonReader::PrintRoute(ostream& out, PreparedStat* s) const {
	Builder request {};
	const std::vector<transport_router::Edges>* edges_data = router_.GetEdgesData();
	auto route_data = router_.GetRoute(s->route.from, s->route.to);
	request.StartDict().Key("request_id"s).Value(s->id);
	if (route_data && route_data->edges.size() > 0) {
	  request.Key("total_time"s).Value(route_data->weight).Key("items").StartArray();
	  for (size_t edge_id : route_data->edges) {
		std::string name {edges_data->at(edge_id).name};
		if (edges_data->at(edge_id).type == edge_type::WAIT) {
		  request.StartDict()
			  .Key("stop_name"s)
			  .Value(name)
			  .Key("time"s)
			  .Value(edges_data->at(edge_id).time)
			  .Key("type"s)
			  .Value("Wait"s)
			  .EndDict();
		} else {
		  request.StartDict()
			  .Key("bus"s)
			  .Value(name)
			  .Key("time"s)
			  .Value(edges_data->at(edge_id).time)
			  .Key("type"s)
			  .Value("Bus"s)
			  .Key("span_count"s)
			  .Value(static_cast<int>(edges_data->at(edge_id).span_count))
			  .EndDict();
		}
	  }
	  request.EndArray();
	} else if (!route_data) {
	  request.Key("error_message"s).Value("not found"s);
	} else {
	  request.Key("total_time"s).Value(0).Key("items").StartArray().EndArray();
	}
	request.EndDict();
	Print(Document {request.Build()}, out);
  }

  void JsonReader::PrintRequests(std::ostream& out, RequestHandler& request_handler) {
	out << "["s << std::endl;
	bool first = true;
	for (const auto& elem : requests_) {
	  if (PreparedStat* s = dynamic_cast<PreparedStat*>(elem.get())) {
		if (!first) {
		  out << ',' << std::endl;
		}
		if (s->type_data == TypeData::STOP) {
		  PrintStop(out, s);
		} else if (s->type_data == TypeData::BUS) {
		  PrintBus(out, s);
		} else if (s->type_data == TypeData::MAP) {
		  PrintMap(out, s, request_handler);
		} else if (s->type_data == TypeData::ROUTE) {
		  PrintRoute(out, s);
		}
		first = false;
	  }
	}
	out << std::endl << "]"s << std::endl;
  }

  void JsonReader::InitDoc(std::istream& in) {
	document_opt_ = json::Load(in);
  }

  void JsonReader::AddBase(const std::vector<Node>& vec) {
	for (const auto& elem : vec) {
	  if (elem.AsDict().count("type"s)) {
		if (elem.AsDict().at("type"s).AsString() == "Stop"s) {
		  requests_.push_back(
			  std::make_unique<PreparedStop>(detail::BaseStop(elem.AsDict())));
		} else if (elem.AsDict().at("type"s).AsString() == "Bus"s) {
		  requests_.push_back(
			  std::make_unique<PreparedBus>(detail::BaseBus(elem.AsDict())));
		}
	  }
	}
  }

  void JsonReader::AddStat(const std::vector<Node>& vec) {
	for (const auto& elem : vec) {
	  if (elem.AsDict().count("type"s)) {
		if (elem.AsDict().at("type"s).AsString() == "Bus"s
			|| elem.AsDict().at("type"s).AsString() == "Stop"s
			|| elem.AsDict().at("type"s).AsString() == "Map"s
			|| elem.AsDict().at("type"s).AsString() == "Route"s) {
		  requests_.emplace_back(
			  std::make_unique<PreparedStat>(detail::Stat(elem.AsDict())));
		}
	  }
	}
  }

  void JsonReader::AddRender(const std::map<std::string, Node>& dic) {
	renderer_.SetSettings(RenderMap(dic));
  }

  void JsonReader::AddRouting(const std::map<std::string, Node>& dic) {
	router_.SetSettings(RouterMap(dic));
  }

  void JsonReader::AddSerialization(const std::map<std::string, Node>& dic) {
	std::visit(
		[&dic](auto&& arg) {
		  using T = typename std::decay<decltype(arg)>::type;
		  if constexpr (std::is_same<T, serial::Serializator>::value) {
			arg.SetSettings(SerializationCatalogue(dic));
		  } else if (std::is_same<T, deserial::DeSerializator>::value) {
			arg.SetSettings(SerializationCatalogue(dic));
		  }
		},
		serialization_);
  }

  void JsonReader::StopStatPrepare(const transport::StopInfo& request,
								   Builder& dict) const {
	auto [name_stop, buses] = request;
	if (name_stop[0] == '!') {
	  dict.Key("error_message"s).Value("not found"s);
	  return;
	}
	if (buses.size() == 0) {
	  dict.Key("buses"s).StartArray().EndArray();
	} else {
	  dict.Key("buses"s).StartArray();
	  for (auto& bus : buses) {
		std::string s_bus(bus);
		dict.Value(std::move(s_bus));
	  }
	  dict.EndArray();
	}
  }

  void JsonReader::BusStatPrepare(const transport::RouteInfo& request,
								  Builder& dict) const {
	if (request.name[0] != '!') {
	  dict.Key("curvature"s)
		  .Value(request.curvature)
		  .Key("route_length"s)
		  .Value(request.route_length)
		  .Key("stop_count"s)
		  .Value(static_cast<int>(request.real_stops_count))
		  .Key("unique_stop_count"s)
		  .Value(static_cast<int>(request.unique_stops_count));
	} else {
	  dict.Key("error_message"s).Value("not found"s);
	}
  }

}  // namespace jsoninputer
