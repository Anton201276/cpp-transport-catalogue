#pragma once

#include <string_view>
#include <deque>
#include <unordered_map>
#include <unordered_set>
#include <set>
#include <algorithm>

#include "domain.h"


namespace transportcatalogue {

	class TransportCatalogue {
	public:
		void AddBusStop(const string& name, Coordinates coordinates);
		void AddBusRoute(const string& name, const vector<string_view>& busroute, bool type);
		void SetBusStopDistance(std::string_view busstop, std::string_view busstop_next, int distance);

		int GetBusStopDistance(std::string_view busstop, std::string_view busstop_next) const;
		BusPtr GetRouteInfo(string_view name) const;
		StopPtr GetBusStopInfo(string_view name) const;
		BusStatistic GetRouteStatistic(string_view name) const;
		const unordered_set<string_view>& GetRouteForBusStop(string_view name) const;
		int GetCountBuses() const;
		const unordered_map<string_view, BusPtr>* GetBusesInfo() const;

	private:
		struct Distance_Hasher {
			size_t operator()(pair<string_view, string_view> p) const noexcept {
				string frt{ p.first };
				string snd{ p.second };
				size_t h1 = std::hash<std::string>{}(frt);
				size_t h2 = std::hash<std::string>{}(snd);
				return h1 * 100 + h2;
			}
		};

	private:
		deque<StopInfo> busstop_info_;
		unordered_map<string_view, StopPtr> ptr_busstop_info_;

		deque<BusInfo> busroute_info_;
		unordered_map<string_view, BusPtr> ptr_busroute_info_;

		unordered_map<string_view, unordered_set<string_view>> ptr_busstop_route_info_;

		unordered_map<pair<string_view, string_view>, int, Distance_Hasher> busstop_distance_info_;
	};
}
