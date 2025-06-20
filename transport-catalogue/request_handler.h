#pragma once

#include <tuple>
#include <memory>
#include <optional>
#include <set>

#include <iostream>

#include "transport_catalogue.h"
#include "map_renderer.h"

using namespace transportcatalogue;
using namespace renderer;

class RequestHandler {
public:
    RequestHandler(const TransportCatalogue& db, const renderer::MapRenderer& renderer) : db_(db),  renderer_(renderer) {};
    std::optional<BusStat> GetBusStat(const std::string_view& bus_name) const;
    const std::vector<BusPtr>* GetBusesByStop(const std::string_view& stop_name) const;
    StopPtr GetStopBusByName(const std::string_view& stop_name) const;
    svg::Document RenderMap() const;

    void AddStatisticsRequest(std::tuple<int, std::string, std::string>& stat_req);
    int GetCountStatisticsRequest(void) const;
    const std::tuple<int, std::string, std::string>& GetRequestByNumber(int id) const;

private:
    const TransportCatalogue& db_;
    const renderer::MapRenderer& renderer_;
    vector<std::tuple<int, std::string, std::string>> statistics_request_{};
};
