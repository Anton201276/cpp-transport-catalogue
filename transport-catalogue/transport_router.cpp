#include "transport_router.h"

std::optional<RouterInfo> TransportRouter::GetGraphRoute(std::string_view route_from, std::string_view route_to) const {
	VertexId from_vertex = vertex_.at(route_from);
	VertexId to_vertex = vertex_.at(route_to);

	std::optional<Router<Weight>::RouteInfo> optim_route = router_->BuildRoute(from_vertex, to_vertex);

	if (!optim_route.has_value()) {
		return std::nullopt;
	}

	std::vector<EdgeInfo> edges;
	std::vector<Weight> weight;

	for (size_t i = 0; i < optim_route.value().edges.size(); ++i) {
		EdgeId id = optim_route.value().edges[i];
		edges.push_back(edge_info_[id]);
		weight.push_back(graph_.GetEdge(id).weight);
	}
	return RouterInfo{ optim_route.value().weight, std::move(edges), std::move(weight) };
}

void TransportRouter::BuildGraphRoute() {
	if (db_.GetCountStops() > 0 && db_.GetCountBuses() > 0) {
		graph_ = DirectedWeightedGraph<Weight>(2 * db_.GetCountStops());
		AddAllStopVertexs(db_);
		AddAllRouterEdges(db_);
		router_ = std::make_unique<Router<Weight>>(graph_);
	}
};

void TransportRouter::AddAllStopVertexs(const TransportCatalogue& db) {
	VertexId vertexId = 0;
	VertexId prev_vertexId = 0;
	Edge edge{ vertexId , prev_vertexId , static_cast<Weight>(routing_settings_.bus_wait_time)};
	for (const auto& [stop, ptr] : db.GetStopsInfo()) {
		WaitEdgeInfo stc_wait{ ptr, prev_vertexId };
		vertex_[stop] = vertexId++;
		edge.from = prev_vertexId;
		edge.to = vertexId++;
		graph_.AddEdge(edge);
		edge_info_.push_back(stc_wait);
		prev_vertexId = vertexId;
	}
}

void TransportRouter::AddAllRouterEdges(const TransportCatalogue& db) {
	const double minutes_by_meter = 0.06 / routing_settings_.bus_velocity;
	Edge edge{ 0 , 0 , 0.0 };
	for (const auto& [bus, ptr] : db.GetBusesInfo()) {

		size_t middle = !ptr->type ? ptr->busstop_info.size() >> 1 : ptr->busstop_info.size();
		
		for (size_t i = 0; i + 1 <  ptr->busstop_info.size(); ++i) {
			double distance = 0;
			edge.from = vertex_[ptr->busstop_info[i]->name] + 1;
			for (size_t j = i + 1; j < ptr->busstop_info.size(); ++j) {
				BusEdgeInfo bus_edge{ ptr , static_cast<int>(j - i)};
				edge.to = vertex_[ptr->busstop_info[j]->name];
				distance += static_cast<double>(db.GetBusStopDistance(ptr->busstop_info[j-1]->name, ptr->busstop_info[j]->name));
				edge.weight = distance * minutes_by_meter;
				graph_.AddEdge(edge);
				edge_info_.push_back(bus_edge);
				if (j == middle) {
					break;
				}
			}
		}
	}
}

const RoutingSettings& TransportRouter::GetRoutingSettings() const {
	return routing_settings_;
}

void TransportRouter::SetRoutingSettings(int wait, int velocity) {
	routing_settings_.SetParams(wait, velocity);
}