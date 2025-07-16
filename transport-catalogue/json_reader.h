#pragma once
#include "json_builder.h"
#include "transport_router.h"
#include "request_handler.h"
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <vector>
#include <set>
#include <algorithm>

using namespace std;
using namespace transportcatalogue;
using namespace json;

//using namespace std::literals;

void LoadTransportCatalogueFromJson(TransportCatalogue& tc, const json::Document& doc);
void AddStatisticsRequestFromJson(RequestHandler& rh, const json::Document& doc);
void PrintAnswerToJson(RequestHandler& rh, std::ostream& output);
Node GetAnswerBusStatistics(const RequestHandler& rh, const int id, const std::string& name);
Node GetAnswerBusesByStop(const RequestHandler& rh, const int id, const std::string& name);
Node GetAnswerRoute(const TransportRouter& rt, const int id, const std::string& from, const std::string& to);
Node GetAnswerSvgMap(const RequestHandler& rh, const int id);
void LoadRendererSettingFromJson(MapRenderer& mr, const json::Document& doc);
void LoadStops(TransportCatalogue& tc, const Array& stop_desc);
void LoadStopsDistance(TransportCatalogue& tc, const Array& stop_desc);
void LoadBuses(TransportCatalogue& tc, const Array& stop_desc);
void LoadRoutingSettings(TransportCatalogue& tc, const Dict& rout_set);
