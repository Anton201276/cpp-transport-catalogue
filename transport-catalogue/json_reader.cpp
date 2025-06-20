#include "json_reader.h"
#include <sstream>

void LoadTransportCatalogueFromJson(TransportCatalogue& tc, const json::Document& doc) {

	Dict top_dict = doc.GetRoot().AsDict();
	Array bus_stop_desc = top_dict["base_requests"].AsArray();

	LoadStops(tc,bus_stop_desc);
	LoadStopsDistance(tc, bus_stop_desc);
	LoadBuses(tc, bus_stop_desc);

}

void LoadStops(TransportCatalogue& tc, const Array& stop_desc) {
	for (size_t j = 0; j < stop_desc.size(); ++j) {
		Dict stop_dict = stop_desc[j].AsDict();
		auto itr = stop_dict.find("type");
		if (itr->second.AsString() == "Stop") {
			const std::string stop_name = stop_dict.at("name").AsString();
			Coordinates coordinates{ stop_dict.at("latitude").AsDouble(), stop_dict.at("longitude").AsDouble() };
			tc.AddBusStop(stop_name, coordinates);
		}
	}
}

void LoadStopsDistance(TransportCatalogue& tc, const Array& stop_desc) {
	for (size_t j = 0; j < stop_desc.size(); ++j) {
		Dict dict = stop_desc[j].AsDict();
		auto itr = dict.find("type");
		if (itr->second.AsString() == "Stop") {
			auto itr_d = dict.find("road_distances");
			if (itr_d != dict.end()) {
				Dict dist_dict = itr_d->second.AsDict();
				for (const auto& dist : dist_dict) {
					const std::string stop_name = dict.at("name").AsString();
					std::string_view stop_next = dist.first;
					int distance = dist.second.AsInt();
					tc.SetBusStopDistance(stop_name, stop_next, distance);
				}
			}
		}
	}
}

void LoadBuses(TransportCatalogue& tc, const Array& stop_desc) {
	for (size_t j = 0; j < stop_desc.size(); ++j) {
		Dict dict = stop_desc[j].AsDict();
		auto itr = dict.find("type");
		if (itr->second.AsString() == "Bus") {
			const std::string bus_name = dict.at("name").AsString();
			Array stops_name = dict.at("stops").AsArray();
			Array tmp_stop(stops_name.begin(), stops_name.end());

			if (!dict.at("is_roundtrip").AsBool()) {
				tmp_stop.insert(tmp_stop.end(), next(stops_name.rbegin()), stops_name.rend());
			}

			vector<string_view> stops_vec;
			for (const auto& sn : tmp_stop) {
				stops_vec.push_back(sn.AsString());
			}
			tc.AddBusRoute(bus_name, stops_vec, dict.at("is_roundtrip").AsBool());
		}
	}
}

void AddStatisticsRequestFromJson(RequestHandler& rh, const json::Document& doc) {
	Dict top_dict = doc.GetRoot().AsDict();
	Array stat_desc = top_dict["stat_requests"].AsArray();

	if (!stat_desc.empty()) {
		for (const auto& arr : stat_desc) {
			Dict dict = arr.AsDict();
			if (dict["type"].AsString() != "Map") {
				std::tuple<int, std::string, std::string> stat_tuple{ dict["id"].AsInt(), dict["type"].AsString(), dict["name"].AsString() };
				rh.AddStatisticsRequest(stat_tuple);
			}
			else {
				std::tuple<int, std::string, std::string> stat_tuple{ dict["id"].AsInt(), dict["type"].AsString(), "" };
				rh.AddStatisticsRequest(stat_tuple);
			}
		}
	}
}

void PrintAnswerToJson(RequestHandler& rh, std::ostream& output) {
	
	int count_req = rh.GetCountStatisticsRequest();
	if (count_req == 0) {
		return;
	}

	json::Builder jb = json::Builder();
	jb.StartArray();

	for (int j = 0; j < count_req; ++j) {
		int id;
		std::string type;
		std::string name;
		std::tie(id, type, name) = rh.GetRequestByNumber(j);
		if (type == "Stop"s) {
			jb.Value(GetAnswerBusesByStop(rh, id, name).GetValue());
		}
		else if (type == "Bus"s) {
			jb.Value(GetAnswerBusStatistics(rh, id, name).GetValue());
		}
		else {
			jb.Value(GetAnswerSvgMap(rh, id).GetValue());
		}
	}
	jb.EndArray();
	json::Document doc(jb.Build());
	Print(doc, output);
}

Node GetAnswerBusesByStop(const RequestHandler& rh, const int id, const std::string& name) {

	json::Builder jb = json::Builder();
	if (rh.GetStopBusByName(name) == nullptr) {
		return jb.StartDict().Key("request_id").Value(id).Key("error_message").Value("not found"s).EndDict().Build();
	}

	jb.StartDict().Key("buses").StartArray();

	const std::vector<BusPtr>* ptr_buses = std::move(rh.GetBusesByStop(name));
	if (ptr_buses != nullptr) {
		vector<BusPtr> buses(ptr_buses->begin(), ptr_buses->end());

		std::sort(buses.begin(), buses.end(), [](BusPtr lhs, BusPtr rhs) {return lhs->name < rhs->name;});


		for (BusPtr ptrbus : buses) {
			jb.Value(ptrbus->name);
		}
	}
	jb.EndArray().Key("request_id").Value(id).EndDict();
	delete ptr_buses;

	return jb.Build();
}

Node GetAnswerBusStatistics(const RequestHandler& rh, const int id, const std::string& name) {
	std::optional<BusStat> busstat = rh.GetBusStat(name);

	json::Builder jb = json::Builder();

	if (!busstat.has_value()) {
		jb.StartDict().Key("request_id"s).Value(id).Key("error_message").Value("not found"s).EndDict();
	}
	else {
		jb.StartDict().Key("curvature").Value(busstat.value().curvature).Key("request_id").Value(id)
			.Key("route_length").Value(busstat.value().distance).Key("stop_count").Value(static_cast<int>(busstat.value().count_stopbus))
			.Key("unique_stop_count").Value(static_cast<int>(busstat.value().uniq_stopbus)).EndDict();
	}

	return jb.Build();
}

Node GetAnswerSvgMap(const RequestHandler& rh, const int id) {
	
	svg::Document doc = rh.RenderMap();
	std::ostringstream s_out;
	doc.Render(s_out);
	std::string print_str;
	print_str = s_out.str();

	json::Builder jb = json::Builder();
	jb.StartDict().Key("map").Value(print_str).Key("request_id").Value(id).EndDict();

	return jb.Build();
}

void LoadRendererSettingFromJson(MapRenderer& mr, const json::Document& doc) {
	Dict top_dict = doc.GetRoot().AsDict();
	Dict render_settings = top_dict["render_settings"].AsDict();
	MapSettings map_settings;

	map_settings.wight = render_settings["width"].AsDouble();
	map_settings.height = render_settings["height"].AsDouble();
	map_settings.padding = render_settings["padding"].AsDouble();
	map_settings.stop_radius = render_settings["stop_radius"].AsDouble();
	map_settings.line_width = render_settings["line_width"].AsDouble();
	map_settings.bus_label_font_size = render_settings["bus_label_font_size"].AsInt();
	map_settings.bus_label_offset[0] = render_settings["bus_label_offset"].AsArray()[0].AsDouble();
	map_settings.bus_label_offset[1] = render_settings["bus_label_offset"].AsArray()[1].AsDouble();
	map_settings.stop_label_font_size = render_settings["stop_label_font_size"].AsInt();
	map_settings.stop_label_offset[0] = render_settings["stop_label_offset"].AsArray()[0].AsDouble();
	map_settings.stop_label_offset[1] = render_settings["stop_label_offset"].AsArray()[1].AsDouble();
	map_settings.underlayer_color = mr.GetColorFromJsonNode(render_settings["underlayer_color"]);
	map_settings.underlayer_width = render_settings["underlayer_width"].AsDouble();

	map_settings.color_palette.clear();

	for (const auto& node : render_settings["color_palette"].AsArray()) {
		map_settings.color_palette.push_back(mr.GetColorFromJsonNode(node));
	}

	mr.SetMapSettings(map_settings);
}