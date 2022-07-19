#pragma once

#include "transport_catalogue.h"
#include "map_renderer.h"
#include "transport_router.h"
#include "serialization.h"

#include <optional>
#include <iostream>

namespace transport {
  class RequestHandler {
  public:
	explicit RequestHandler(
		const TransportCatalogue& db, map_renderer::MapRenderer& renderer,
		transport_router::TransportRouter& router,
		std::variant<serial::Serializator, deserial::DeSerializator>& serialization)
		: db_(db), renderer_(renderer), router_(router), serialization_(serialization) {}

	std::optional<const Bus*> GetBusStat(std::string_view bus_name);

	const std::set<std::string_view>* GetBusesByStop(
		const std::string_view& stop_name) const;

	void RenderMap() const;
	void SetCatalogueDataToRender() const;

	void GenerateRouter() const;

	void SerializeBase() const;
	void DeserializeBase() const;

  private:
	void SetStopsForRender() const;
	void SetRoutesForRender() const;

  private:
	const TransportCatalogue& db_;
	map_renderer::MapRenderer& renderer_;
	transport_router::TransportRouter& router_;
	std::variant<serial::Serializator, deserial::DeSerializator>& serialization_;
  };
}  // namespace transport
