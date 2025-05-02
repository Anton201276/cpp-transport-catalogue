#pragma once

#include <string_view>
#include <deque>
#include <unordered_map>
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

	struct BusRoutInfo {
		string name;
		vector<const BusStopInfo*> busstopinfo;
	};

	struct BusRoutStatistic {
		size_t countstopbus = 0;
		size_t uniqstopbus = 0;
		double lenght = 0;
	};

	class TransportCatalogue {
	public:
		void AddBusStop(const string& name, Coordinates coordinates);
		void AddBusRout(const string& name, vector<string_view> busroute);

		const BusRoutInfo* GetRouteInfo(string_view name) const;
		const BusStopInfo* GetBusStopInfo(string_view name) const;
		BusRoutStatistic GetRouteStatistic(string_view name) const;
		vector<string_view> GetRouteForBusStop(string_view name) const;

	private:
		deque<BusStopInfo> busstopinfo_;
		unordered_map<string_view, const BusStopInfo*> ptr_busstopinfo_;

		deque<BusRoutInfo> busroutinfo_;
		unordered_map<string_view, const BusRoutInfo*> ptr_busroutinfo_;

		unordered_map<string_view, vector<string_view>> ptr_busstoproutinfo_;
	};
}
