#pragma once

#include <string>
#include <vector>
#include "geo.h"

using namespace std;
using namespace geo;

namespace transportcatalogue {

	struct StopInfo {
		string name;
		Coordinates coordinates;
	};

	struct BusInfo {
		string name;
		vector<const StopInfo*> busstop_info;
		bool type = false;
	};

	struct BusStatistic {
		size_t count_stopbus = 0;
		size_t uniq_stopbus = 0;
		double lenght = 0;
		int distance = 0;
		double curvature = 0.0;
	};

	struct RoutingSettings {
		int bus_wait_time = 0;
		int bus_velocity = 0;
		void SetParams(int wait, int velocity) {
			bus_wait_time = wait > 0 ? wait : 0;
			bus_velocity = velocity > 0 ? velocity : 0;
		}
	};

	using BusPtr = const BusInfo*;
	using StopPtr = const StopInfo*;
	using BusStat = BusStatistic;
	inline bool RoundTrip = true;
	inline bool  NotRoundTrip = false;
	using Weight = double;
}