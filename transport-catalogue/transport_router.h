#pragma once
#include "json_builder.h"
#include "transport_catalogue.h"
#include "router.h"

#include <string_view>
#include <memory>


using namespace graph;
using namespace transportcatalogue;

struct BusEdgeInfo {
	BusPtr bus_ptr;
	int span_count;
};

struct WaitEdgeInfo {
	StopPtr stop_ptr;
	VertexId id;
};

using EdgeInfo = std::variant<BusEdgeInfo, WaitEdgeInfo>;

struct RouterInfo {
	Weight weight;
	std::vector<EdgeInfo> edges;
	std::vector<Weight> edges_weight;
};

class TransportRouter {
public:
	TransportRouter(const TransportCatalogue& db) : db_(db) {};

	void BuildGraphRoute();
	std::optional<RouterInfo> GetGraphRoute(std::string_view, std::string_view) const;
	const RoutingSettings& GetRoutingSettings() const;
	void SetRoutingSettings(int wait, int velocity);

private:
	void AddAllStopVertexs(const TransportCatalogue& db);
	void AddAllRouterEdges(const TransportCatalogue& db);
private:
	const TransportCatalogue& db_;
	RoutingSettings routing_settings_;
	DirectedWeightedGraph<Weight> graph_;
	std::unordered_map<std::string_view, VertexId> vertex_;
	std::vector<EdgeInfo> edge_info_;
	std::unique_ptr<Router<Weight>> router_;
};