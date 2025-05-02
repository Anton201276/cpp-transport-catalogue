#include "stat_reader.h"

void ParseAndPrintStat(const TransportCatalogue& tansport_catalogue, std::string_view request,
    std::ostream& output) {

    if (request.find("Bus") != request.npos) {
        size_t idx = request.find_first_not_of(' ', request.find_first_of('s') + 1);
        size_t dst = request.find_last_not_of(' ') - idx + 1;
        std::string_view busroute = request.substr(idx, dst);

        BusRoutStatistic statistic = tansport_catalogue.GetRouteStatistic(busroute);

        output << std::setprecision(6);

        if (statistic.countstopbus == 0) {
            output << "Bus "s << busroute << ": not found" << "\n";
            return;
        }

        output << "Bus "s << busroute << ": "s << statistic.countstopbus << " stops on route, "s << statistic.uniqstopbus
            << " unique stops, "s << statistic.lenght << " route length"s << "\n";
        return;
    }

    if (request.find("Stop") != request.npos) {
        size_t idx = request.find_first_not_of(' ', request.find_first_of('p') + 1);
        size_t dst = request.find_last_not_of(' ') - idx + 1;
        std::string_view busroute = request.substr(idx, dst);

        vector<string_view> statistic = tansport_catalogue.GetRouteForBusStop(busroute);

        if (statistic.back() == "no buses") {
            output << "Stop "s << busroute << ": no buses" << "\n";
        }
        else if (statistic.back() == "not found") {
            output << "Stop "s << busroute << ": not found" << "\n";
        }
        else {
            output << "Stop "s << busroute << ": buses";
            for (auto it = statistic.begin(); it != statistic.end(); ++it) {
                output << " " << *it;
            }
            output << "\n";
        }
    }
}