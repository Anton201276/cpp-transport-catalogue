#include <iostream>
#include "request_handler.h"
#include "json_reader.h"
#include "map_renderer.h"

using namespace std::literals;

int main() {

    TransportCatalogue catalogue;
    MapRenderer render;
    TransportRouter router(catalogue);
    RequestHandler request_handler(catalogue, router, render);

    try {
        json::Document doc = Load(std::cin);
        LoadTransportDataFromJson(catalogue, router, doc);
        LoadRendererSettingFromJson(render, doc);
        AddStatisticsRequestFromJson(request_handler, doc);
    }
    catch (...) {
        std::cerr << "Ошибка ввода json-файла"sv << std::endl;
        std::cin.clear();
        return 0;
    }
    PrintAnswerToJson(request_handler, std::cout);
}