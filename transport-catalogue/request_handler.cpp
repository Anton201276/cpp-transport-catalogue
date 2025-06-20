#include "request_handler.h"

using namespace transportcatalogue;

void RequestHandler::AddStatisticsRequest(std::tuple<int, std::string, std::string>& stat_req) {
	statistics_request_.push_back(std::move(stat_req));
}

int RequestHandler::GetCountStatisticsRequest(void) const {
	return statistics_request_.size();
}

const std::vector<BusPtr>* RequestHandler::GetBusesByStop(const std::string_view& stop_name) const {
	const std::vector<BusPtr>* ptr_buses = nullptr;

	const unordered_set<string_view>& buses_by_stop_string = db_.GetRouteForBusStop(stop_name);
	if (buses_by_stop_string.empty()) {
		return ptr_buses;
	}

	std::vector<BusPtr> buses_by_stop_set;
	for (const auto str : buses_by_stop_string) {
		buses_by_stop_set.push_back(db_.GetRouteInfo(str));
	}
	
	try {
		ptr_buses = new vector<BusPtr>(std::move(buses_by_stop_set));
	}
	catch (const std::exception&) {
		
	}
	return ptr_buses;
}

const std::tuple<int, std::string, std::string>& RequestHandler::GetRequestByNumber(int id) const {
	static const std::tuple<int, std::string, std::string> tmp_tuple{};
	if (id < static_cast<int>(statistics_request_.size())) {
		return statistics_request_[id];
	}
	return tmp_tuple;
}

std::optional<BusStat> RequestHandler::GetBusStat(const std::string_view& bus_name) const {
	BusStat busstat = db_.GetRouteStatistic(bus_name);
	if (busstat.count_stopbus == 0) {
		return std::nullopt;
	}
	return busstat;
}

StopPtr RequestHandler::GetStopBusByName(const std::string_view& stop_name) const {
	return db_.GetBusStopInfo(stop_name);
}

svg::Document RequestHandler::RenderMap() const {
	svg::Document doc;

	vector<string_view> buses_name;
	const unordered_map<string_view, BusPtr>* ptr_buses_info = db_.GetBusesInfo();

	for (const auto& [str, ptr] : *ptr_buses_info) {
		if (!ptr->busstop_info.empty()) {
			buses_name.push_back(str);
		}
	}
	std::sort(buses_name.begin(), buses_name.end());

	vector<BusPtr> buses_info;
	for (const auto str : buses_name) {
		buses_info.push_back(db_.GetRouteInfo(str));
	}

	set<std::string_view> stop_name;
	for (auto& bus : buses_info) {
		for (auto& stop : bus->busstop_info) {
			stop_name.insert(stop->name);
		}
	}

	vector<StopPtr> stops_info;
	for (auto& stop : stop_name) {
		stops_info.push_back(GetStopBusByName(stop));
	}

	doc = renderer_.MapRenderBusTrip(buses_info, stops_info);
	return doc;
}