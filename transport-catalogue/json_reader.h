#pragma once
#include "json.h"
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

void LoadTransportCatalogueFromJson(TransportCatalogue& tc, const json::Document& doc);
void AddStatisticsRequestFromJson(RequestHandler& rh, const json::Document& doc);
void PrintAnswerToJson(RequestHandler& rh, std::ostream& output);
Dict NodeAnswerBusStatistics(const RequestHandler& rh, const int id, const std::string& name);
Dict NodeAnswerBusesByStop(const RequestHandler& rh, const int id, const std::string& name);
Dict SvgAnswerRenderMap(const RequestHandler& rh, const int id);
void LoadRendererSettingFromJson(MapRenderer& mr, const json::Document& doc);



