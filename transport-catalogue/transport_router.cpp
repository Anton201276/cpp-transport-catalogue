#include "transport_router.h"

std::optional<Router<Weight>::RouteInfo> TransportRouter::GetRoute(std::string_view route_from, std::string_view route_to) const {
	
	VertexId from_vertex = vertex_.at(route_from);
	VertexId to_vertex = vertex_.at(route_to);

	return router_->BuildRoute(from_vertex, to_vertex);
}

void TransportRouter::AddAllStopVertexs(const TransportCatalogue& db) {
	VertexId vertexId = 0;
	VertexId prev_vertexId = 0;
	Edge edge{ vertexId , prev_vertexId , static_cast<Weight>(db.GetRoutingSettings().bus_wait_time)};
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
	const double minutes_by_meter = 0.06 / db.GetRoutingSettings().bus_velocity;
	Edge edge{ 0 , 0 , 0.0 };
	for (const auto& [bus, ptr] : db.GetBusesInfo()) {

		size_t middle = !ptr->type ? ptr->busstop_info.size() >> 1 : ptr->busstop_info.size();
		
		for (size_t i = 0; i <  ptr->busstop_info.size() - 1; ++i) {
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
