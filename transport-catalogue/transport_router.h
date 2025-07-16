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

class TransportRouter {
public:
	TransportRouter(const TransportCatalogue& db) {
		graph_ = DirectedWeightedGraph<Weight>(2 * db.GetCountStops());
		AddAllStopVertexs(db);
		AddAllRouterEdges(db);
		router_ = std::make_unique<Router<Weight>>(graph_);
	};

	std::optional<Router<Weight>::RouteInfo> GetRoute(std::string_view, std::string_view) const;
	const DirectedWeightedGraph<Weight>& GetGraph() const {
		return graph_;
	}

	const std::vector<EdgeInfo>& GetEdges() const {
		return edge_info_;
	}

private:
	void AddAllStopVertexs(const TransportCatalogue& db);
	void AddAllRouterEdges(const TransportCatalogue& db);
private:
	DirectedWeightedGraph<Weight> graph_;
	std::unordered_map<std::string_view, VertexId> vertex_;
	std::vector<EdgeInfo> edge_info_;
	std::unique_ptr<Router<Weight>> router_;
};