#pragma once

#include <iosfwd>
#include <string_view>
#include <iomanip>
#include "transport_catalogue.h"

using namespace transportcatalogue;

void ParseAndPrintStat(const TransportCatalogue& tansport_catalogue, std::string_view request,
    std::ostream& output);
