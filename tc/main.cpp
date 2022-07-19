#include <fstream>
#include <iostream>
#include <random>
#include <string_view>

#include "json_reader.h"
#include "request_handler.h"

struct Visitor {
  void operator()(serial::Serializator s) {
	s.Serialize();
  }
  void operator()(deserial::DeSerializator d) {
	d.DeSerialize();
  }
};

void SerializeBase(std::istream& input) {
  transport::TransportCatalogue catalog;
  map_renderer::MapRenderer renderer;
  transport_router::TransportRouter router(catalog);
  std::variant<serial::Serializator, deserial::DeSerializator> serialization(
	  serial::Serializator(catalog, renderer, router));
  jsoninputer::JsonReader json(catalog, renderer, router, serialization);
  transport::RequestHandler request_handler(catalog, renderer, router, serialization);
  json.ReadInput(input);
  json.AddCatalogue();
  request_handler.SerializeBase();
}

void DeserializeBase(std::istream& input, std::ostream& output) {
  transport::TransportCatalogue catalog;
  map_renderer::MapRenderer renderer;
  transport_router::TransportRouter router(catalog);
  std::variant<serial::Serializator, deserial::DeSerializator> deserialization(
	  deserial::DeSerializator(catalog, renderer, router));
  jsoninputer::JsonReader json(catalog, renderer, router, deserialization);
  transport::RequestHandler request_handler(catalog, renderer, router, deserialization);
  json.ReadInput(input);
  std::visit(
	  [](auto&& arg) {
		using T = typename std::decay<decltype(arg)>::type;
		if constexpr (std::is_same<T, deserial::DeSerializator>::value) {
		  arg.DeSerialize();
		}
	  },
	  deserialization);
  json.PrintRequests(output, request_handler);
}

using namespace std::literals;

void PrintUsage(std::ostream& stream = std::cerr) {
  stream << "Usage: transport_catalogue [make_base|process_requests]\n"sv;
}

int main(int argc, char* argv[]) {
  if (argc != 2) {
	PrintUsage();
	return 1;
  }
  const std::string_view mode(argv[1]);
  if (mode == "make_base"sv) {
	SerializeBase(std::cin);
  } else if (mode == "process_requests"sv) {
	DeserializeBase(std::cin, std::cout);
  } else {
	PrintUsage();
	return 1;
  }
}
