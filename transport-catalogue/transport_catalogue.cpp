#include "transport_catalogue.h"

namespace transportcatalogue {

    void TransportCatalogue::AddBusStop(const string& name, Coordinates coordinates) {

        if (name.empty()) {
            return;
        }

        BusStopInfo sbusstopinfo{ name , coordinates };

        busstopinfo_.push_back(move(sbusstopinfo));
        ptr_busstopinfo_[busstopinfo_.back().name] = &busstopinfo_.back();
    }

    void TransportCatalogue::AddBusRout(const string& name, vector<string_view> busroute) {

        if (name.empty() || busroute.empty()) {
            return;
        }
        BusRoutInfo sbusrouteinfo{ name, {} };

        for (auto itr = busroute.begin(); itr != busroute.end(); ++itr) {
            sbusrouteinfo.busstopinfo.push_back(ptr_busstopinfo_[*itr]);
        }

        busroutinfo_.push_back(move(sbusrouteinfo));
        ptr_busroutinfo_[busroutinfo_.back().name] = &busroutinfo_.back();

        for (auto itr = busroute.begin(); itr != busroute.end(); ++itr) {
            ptr_busstoproutinfo_[ptr_busstopinfo_[*itr]->name].push_back(busroutinfo_.back().name);
        }
    }

    const BusRoutInfo* TransportCatalogue::GetRouteInfo(string_view name) const {
        if (ptr_busroutinfo_.count(name)) {
            return ptr_busroutinfo_.at(name);
        }
        return nullptr;
    }

    const BusStopInfo* TransportCatalogue::GetBusStopInfo(string_view name) const {
        if (ptr_busstopinfo_.count(name)) {
            return ptr_busstopinfo_.at(name);
        }
        return nullptr;
    }

    BusRoutStatistic TransportCatalogue::GetRouteStatistic(string_view name) const {
        const BusRoutInfo* ptr = GetRouteInfo(name);
        BusRoutStatistic retinfo;

        if (ptr == nullptr) {
            return retinfo;
        }

        set<string_view> qname;

        for (auto vname : ptr->busstopinfo) {
            qname.insert(vname->name);
        }

        retinfo.countstopbus = ptr->busstopinfo.size();
        retinfo.uniqstopbus = qname.size();
        double lenghtroute = 0.0;

        for (auto id = 0; id < ptr->busstopinfo.size() - 1; ++id) {
            lenghtroute += ComputeDistance(ptr->busstopinfo[id]->coordinates, ptr->busstopinfo[id + 1]->coordinates);
        }
        retinfo.lenght = lenghtroute;
        return retinfo;
    }

    vector<string_view> TransportCatalogue::GetRouteForBusStop(string_view name) const {
        vector<string_view> vecroutename{};

        if (ptr_busstopinfo_.count(name)) {
            if (ptr_busstoproutinfo_.count(name)) {
                set<string_view> setroutename{};
                for (auto vname : ptr_busstoproutinfo_.at(name)) {
                    setroutename.insert(vname);
                }

                for (auto it = setroutename.begin(); it != setroutename.end(); ++it) {
                    vecroutename.push_back(*it);
                }
                sort(vecroutename.begin(), vecroutename.end());
            }
            else {
                vecroutename.push_back("no buses"sv);
            }
        }
        else {
            vecroutename.push_back("not found"sv);
        }
        return vecroutename;
    }
}
