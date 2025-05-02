#include "transport_catalogue.h"

namespace transportcatalogue {

    void TransportCatalogue::AddBusStop(const string& name, Coordinates coordinates) {

        if (name.empty()) {
            return;
        }

        BusStopInfo sbusstopinfo{ name , coordinates };

        busstop_info_.push_back(move(sbusstopinfo));
        ptr_busstop_info_[busstop_info_.back().name] = &busstop_info_.back();
    }

    void TransportCatalogue::AddBusRoute(const string& name, const vector<string_view>& busroute) {

        if (name.empty() || busroute.empty()) {
            return;
        }
        BusRouteInfo sbusrouteinfo{ name, {} };

        for (auto itr = busroute.begin(); itr != busroute.end(); ++itr) {
            sbusrouteinfo.busstop_info.push_back(ptr_busstop_info_[*itr]);
        }

        busroute_info_.push_back(move(sbusrouteinfo));
        ptr_busroute_info_[busroute_info_.back().name] = &busroute_info_.back();

        for (auto itr = busroute.begin(); itr != busroute.end(); ++itr) {
            ptr_busstop_route_info_[ptr_busstop_info_[*itr]->name].insert(busroute_info_.back().name);
        }
    }

    const BusRouteInfo* TransportCatalogue::GetRouteInfo(string_view name) const {
        auto itr = find_if(ptr_busroute_info_.begin(), ptr_busroute_info_.end(), [=](const std::pair < const string_view, const BusRouteInfo*> brstr) { return brstr.first == name;});
        if (itr != ptr_busroute_info_.end()) {
            return itr->second;
        }
        return nullptr;
    }

    const BusStopInfo* TransportCatalogue::GetBusStopInfo(string_view name) const {
        auto itr = find_if(ptr_busstop_info_.begin(), ptr_busstop_info_.end(), [=](const std::pair<const string_view, const BusStopInfo*> brstr) { return brstr.first == name;});
        if (itr != ptr_busstop_info_.end()) {
            return itr->second;
        }
        return nullptr;
    }

    BusRouteStatistic TransportCatalogue::GetRouteStatistic(string_view name) const {
        const BusRouteInfo* ptr = GetRouteInfo(name);
        BusRouteStatistic retinfo;

        if (ptr == nullptr) {
            return retinfo;
        }

        set<string_view> qname;

        for (auto vname : ptr->busstop_info) {
            qname.insert(vname->name);
        }

        retinfo.countstopbus = ptr->busstop_info.size();
        retinfo.uniqstopbus = qname.size();
        double lenghtroute = 0.0;

        for (size_t id = 0; id < ptr->busstop_info.size() - 1; ++id) {
            lenghtroute += ComputeDistance(ptr->busstop_info[id]->coordinates, ptr->busstop_info[id + 1]->coordinates);
        }
        retinfo.lenght = lenghtroute;
        return retinfo;
    }

    const vector<string_view>& TransportCatalogue::GetRouteForBusStop(string_view name) const {
        static vector<string_view> vecroutename{};
        if (ptr_busstop_route_info_.count(name)) {
            for (auto it = ptr_busstop_route_info_.at(name).begin(); it != ptr_busstop_route_info_.at(name).end(); ++it) {
                    vecroutename.push_back(*it);
            }
            sort(vecroutename.begin(), vecroutename.end());
        }
        return vecroutename;
    }
}
