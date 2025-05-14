#include "input_reader.h"

#include <algorithm>
#include <cassert>
#include <iterator>



/**
 * Парсит строку вида "10.123,  -30.1837" и возвращает пару координат (широта, долгота)
 */
Coordinates ParseCoordinates(std::string_view str) {
    static const double nan = std::nan("");

    auto not_space = str.find_first_not_of(' ');
    auto comma = str.find(',');

    if (comma == str.npos) {
        return { nan, nan };
    }

    auto not_space2 = str.find_first_not_of(' ', comma + 1);

    double lat = std::stod(std::string(str.substr(not_space, comma - not_space)));
    double lng = std::stod(std::string(str.substr(not_space2)));

    return { lat, lng };
}

/**
 * Удаляет пробелы в начале и конце строки
 */
std::string_view Trim(std::string_view string) {
    const auto start = string.find_first_not_of(' ');
    if (start == string.npos) {
        return {};
    }
    return string.substr(start, string.find_last_not_of(' ') + 1 - start);
}

std::pair<std::string_view, std::string_view> ParseDescription(std::string_view str) {
    std::string_view description = Trim(str);
    std::pair<std::string_view, std::string_view> ret_pair;

    auto comma = description.find(',');
    comma = description.find(',', comma + 1);
    if (comma != description.npos) {
        ret_pair.first = description.substr(0, comma); //description.find_last_not_of(' ', comma - 1);
        auto not_space = description.find_first_not_of(' ', comma + 1);
        ret_pair.second = description.substr(not_space, description.size() - not_space + 1);
    }
    else {
        ret_pair.first = description;
        ret_pair.second = "";
    }
    
    return ret_pair;
}

std::vector<std::pair<std::string_view, int>> ParseBusStopDistance(std::string_view str) {
    if (str.empty()) {
        return {};
    }
    std::vector<std::pair<std::string_view, int>> ret_vector;
    bool isParse = false;
    do {
        auto char_m = str.find_first_of('m');
        int meters = std::stoi(std::string(str.substr(0, char_m)));

        auto comma = str.find(',');
        comma = comma != str.npos ? comma : str.size();
        auto not_space_start = str.find_first_not_of(' ', 2 + str.find("to"));
        auto not_space_end = str.find_last_not_of(' ', comma - 1);
        std::string_view busstop = str.substr(not_space_start, not_space_end - not_space_start + 1);
        ret_vector.emplace_back(std::pair<std::string_view, int>{ busstop,meters });

        if (comma != str.size()) {
            str = str.substr(str.find_first_not_of(' ', comma + 1));
            isParse = true;
        }
        else {
            isParse = false;
        }
    } while (isParse);

    return ret_vector;
}

/**
 * Разбивает строку string на n строк, с помощью указанного символа-разделителя delim
 */
std::vector<std::string_view> Split(std::string_view string, char delim) {
    std::vector<std::string_view> result;

    size_t pos = 0;
    while ((pos = string.find_first_not_of(' ', pos)) < string.length()) {
        auto delim_pos = string.find(delim, pos);
        if (delim_pos == string.npos) {
            delim_pos = string.size();
        }
        if (auto substr = Trim(string.substr(pos, delim_pos - pos)); !substr.empty()) {
            result.push_back(substr);
        }
        pos = delim_pos + 1;
    }

    return result;
}

/**
 * Парсит маршрут.
 * Для кольцевого маршрута (A>B>C>A) возвращает массив названий остановок [A,B,C,A]
 * Для некольцевого маршрута (A-B-C-D) возвращает массив названий остановок [A,B,C,D,C,B,A]
 */
std::vector<std::string_view> ParseRoute(std::string_view route) {
    if (route.find('>') != route.npos) {
        return Split(route, '>');
    }

    auto stops = Split(route, '-');
    std::vector<std::string_view> results(stops.begin(), stops.end());
    results.insert(results.end(), std::next(stops.rbegin()), stops.rend());

    return results;
}

CommandDescription ParseCommandDescription(std::string_view line) {
    auto colon_pos = line.find(':');
    if (colon_pos == line.npos) {
        return {};
    }

    auto space_pos = line.find(' ');
    if (space_pos >= colon_pos) {
        return {};
    }

    auto not_space = line.find_first_not_of(' ', space_pos);
    if (not_space >= colon_pos) {
        return {};
    }

    return { std::string(line.substr(0, space_pos)),
            std::string(line.substr(not_space, colon_pos - not_space)),
            std::string(line.substr(colon_pos + 1)) };
}

void InputReader::ParseLine(std::string_view line) {
    auto command_description = ParseCommandDescription(line);
    if (command_description) {
        commands_.push_back(std::move(command_description));
    }
}

void InputReader::ApplyCommands([[maybe_unused]] TransportCatalogue& catalogue) const {

    std::unordered_map<std::string_view, std::string_view> busstop_dist;
    for (auto itr = commands_.begin(); itr != commands_.end(); ++itr) {
        if (itr->command == "Stop") {
            std::pair<std::string_view, std::string_view> pair_desc = ParseDescription(itr->description);
            catalogue.AddBusStop(itr->id, ParseCoordinates(pair_desc.first));
            busstop_dist[itr->id] = pair_desc.second;
        }
    }

    for (auto itr = commands_.begin(); itr != commands_.end(); ++itr) {
        if (itr->command == "Stop") {
            std::vector<std::pair<std::string_view, int>> bus_distance = ParseBusStopDistance(busstop_dist[itr->id]);
            for (std::pair<std::string_view, int>& pair_value : bus_distance) {
                catalogue.SetBusStopDistance(itr->id, pair_value.first, pair_value.second);
            }
        }
    }

    for (auto itr = commands_.begin(); itr != commands_.end(); ++itr) {
        if (itr->command == "Bus") {
            catalogue.AddBusRoute(itr->id, ParseRoute(itr->description));
        }
    }
}