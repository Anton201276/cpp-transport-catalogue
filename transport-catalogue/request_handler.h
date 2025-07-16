#pragma once

#include <tuple>
#include <memory>
#include <optional>
#include <set>
//#include <unordered_map>

#include <iostream>

#include "transport_catalogue.h"
#include "transport_router.h"
#include "map_renderer.h"

using namespace transportcatalogue;
using namespace renderer;

class RequestHandler {
public:
    //RequestHandler(const TransportCatalogue& db, const TransportRouter& router, const renderer::MapRenderer& renderer) 
    RequestHandler(const TransportCatalogue& db, const TransportRouter& router, const renderer::MapRenderer& renderer)
        : db_(db),
          router_(router),
          renderer_(renderer) {};

    std::optional<BusStat> GetBusStat(const std::string_view& bus_name) const;
    const std::vector<BusPtr>* GetBusesByStop(const std::string_view& stop_name) const;
    StopPtr GetStopBusByName(const std::string_view& stop_name) const;
    svg::Document RenderMap() const;

    void AddStatisticsRequest(std::tuple<int, std::string, std::string>& stat_req);
    void AddStopTo(int, std::string);
    int GetCountStatisticsRequest(void) const;
    const std::tuple<int, std::string, std::string>& GetRequestByNumber(int id) const;
    const unordered_map<string_view, BusPtr>& GetBusesInfo() const;
    const std::string& GetStopToById(int) const;

    const TransportCatalogue& GetTransportCatalogue() const {
        return db_;
    }

    const TransportRouter& GetTransportRouter() const {
        return router_;
    }

private:
    const TransportCatalogue& db_;
    const TransportRouter& router_;
    const renderer::MapRenderer& renderer_;
    vector<std::tuple<int, std::string, std::string>> statistics_request_{};
    unordered_map<int, std::string> stop_destination_;
};
