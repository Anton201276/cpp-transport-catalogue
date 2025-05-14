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
		size_t count_stopbus = 0;
		size_t uniq_stopbus = 0;
		double lenght = 0;
		int distance = 0;
		double curvature = 0.0;
	};

	class TransportCatalogue {
	public:
		void AddBusStop(const string& name, Coordinates coordinates);
		void AddBusRoute(const string& name, const vector<string_view>& busroute);
		void SetBusStopDistance(std::string_view busstop, std::string_view busstop_next, int distance);

		const int GetBusStopDistance(std::string_view busstop, std::string_view busstop_next) const;
		const BusRouteInfo* GetRouteInfo(string_view name) const;
		const BusStopInfo* GetBusStopInfo(string_view name) const;
		BusRouteStatistic GetRouteStatistic(string_view name) const;
		const unordered_set<string_view>& GetRouteForBusStop(string_view name) const;

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
		deque<BusStopInfo> busstop_info_;
		unordered_map<string_view, const BusStopInfo*> ptr_busstop_info_;

		deque<BusRouteInfo> busroute_info_;
		unordered_map<string_view, const BusRouteInfo*> ptr_busroute_info_;

		unordered_map<string_view, unordered_set<string_view>> ptr_busstop_route_info_;

		unordered_map<pair<string_view, string_view>, int, Distance_Hasher> busstop_distance_info_;
	};
}
