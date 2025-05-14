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

    void TransportCatalogue::SetBusStopDistance(std::string_view busstop, std::string_view busstop_next, int distance) {
        if (busstop.empty() || busstop_next.empty() || distance == 0) {
            return;
        }
        std::string_view busstop_name = ptr_busstop_info_[busstop]->name;
        std::string_view distance_name = ptr_busstop_info_[busstop_next]->name;
        std::pair<string_view, string_view> p_tmp{ move(busstop_name), move(distance_name) };
        busstop_distance_info_[move(p_tmp)] = distance;
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
        auto itr = ptr_busroute_info_.find(name);
        if (itr != ptr_busroute_info_.end()) {
            return itr->second;
        }
        return nullptr;
    }

    const BusStopInfo* TransportCatalogue::GetBusStopInfo(string_view name) const {        
        auto itr = ptr_busstop_info_.find(name);
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

        retinfo.count_stopbus = ptr->busstop_info.size();
        retinfo.uniq_stopbus = qname.size();
        double lenghtroute = 0.0;
        int distance = 0;

        for (size_t id = 0; id < ptr->busstop_info.size() - 1; ++id) {
            lenghtroute += ComputeDistance(ptr->busstop_info[id]->coordinates, ptr->busstop_info[id + 1]->coordinates);
            pair<string_view, string_view> bs_pair{ ptr->busstop_info[id]->name , ptr->busstop_info[id + 1]->name };
            auto itr = busstop_distance_info_.find(bs_pair);
            if (itr != busstop_distance_info_.end()) {
                distance += itr->second;
            }
            else {
                bs_pair.first = ptr->busstop_info[id + 1]->name;
                bs_pair.second = ptr->busstop_info[id]->name;
                auto itr = busstop_distance_info_.find(bs_pair);
                if (itr != busstop_distance_info_.end()) {
                    distance += itr->second;
                }
            }
        }
        retinfo.distance = distance;
        retinfo.lenght = lenghtroute;
        retinfo.curvature = static_cast<double>(distance) / lenghtroute;
        return retinfo;
    }

    const unordered_set<string_view>& TransportCatalogue::GetRouteForBusStop(string_view name) const {
        static const unordered_set<string_view> return_route_name{};
        if (ptr_busstop_route_info_.count(name)) {
            return ptr_busstop_route_info_.at(name);
        }
        else {
            return return_route_name;
        }
    }

    const int TransportCatalogue::GetBusStopDistance(std::string_view busstop, std::string_view busstop_next) const {
        std::pair<string_view, string_view> p_tmp{ move(busstop), move(busstop_next) };
        return busstop_distance_info_.at(move(p_tmp));
    }
}
