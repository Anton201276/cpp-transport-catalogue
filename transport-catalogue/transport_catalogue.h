#pragma once

#include <string_view>
#include <deque>
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <vector>
#include <set>
#include <algorithm>

#include "geo.h"

using namespace std;

namespace transportcatalogue {

	struct BusStopInfo {
		string name;
		Coordinates coordinates;
	};

	struct BusRouteInfo {
		string name;
		vector<const BusStopInfo*> busstop_info;
	};

	struct BusRouteStatistic {
		size_t countstopbus = 0;
		size_t uniqstopbus = 0;
		double lenght = 0;
	};

	class TransportCatalogue {
	public:
		void AddBusStop(const string& name, Coordinates coordinates);
		void AddBusRoute(const string& name, const vector<string_view>& busroute);

		const BusRouteInfo* GetRouteInfo(string_view name) const;
		const BusStopInfo* GetBusStopInfo(string_view name) const;
		BusRouteStatistic GetRouteStatistic(string_view name) const;
		const vector<string_view>& GetRouteForBusStop(string_view name) const;

	private:
		deque<BusStopInfo> busstop_info_;
		unordered_map<string_view, const BusStopInfo*> ptr_busstop_info_;

		deque<BusRouteInfo> busroute_info_;
		unordered_map<string_view, const BusRouteInfo*> ptr_busroute_info_;

		unordered_map<string_view, unordered_set<string_view>> ptr_busstop_route_info_;
	};
}
