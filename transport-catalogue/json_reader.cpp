#include "json_reader.h"
#include <iostream>

void LoadTransportCatalogueFromJson(TransportCatalogue& tc, const json::Document& doc) {

	Dict top_dict = doc.GetRoot().AsMap();
	Array bus_stop_desc = top_dict["base_requests"].AsArray();

	for (size_t j = 0; j < bus_stop_desc.size(); ++j) {
		Dict stop_dict = bus_stop_desc[j].AsMap();
		auto itr = stop_dict.find("type");
		if (itr->second.AsString() == "Stop") {
			const std::string stop_name = stop_dict.at("name").AsString();
			Coordinates coordinates{ stop_dict.at("latitude").AsDouble(), stop_dict.at("longitude").AsDouble() };
			tc.AddBusStop(stop_name, coordinates);
		}
	}

	for (size_t j = 0; j < bus_stop_desc.size(); ++j) {
		Dict dict = bus_stop_desc[j].AsMap();
		auto itr = dict.find("type");
		if (itr->second.AsString() == "Stop") {
			auto itr_d = dict.find("road_distances");
			if (itr_d != dict.end()) {
				Dict dist_dict = itr_d->second.AsMap();
				for (const auto& dist : dist_dict) {
					const std::string stop_name = dict.at("name").AsString();
					std::string_view stop_next = dist.first;
					int distance = dist.second.AsInt();
					tc.SetBusStopDistance(stop_name, stop_next, distance);
				}
			}
		}
		else {
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
	Dict top_dict = doc.GetRoot().AsMap();
	Array stat_desc = top_dict["stat_requests"].AsArray();

	if (!stat_desc.empty()) {
		for (const auto& arr : stat_desc) {
			Dict dict = arr.AsMap();
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

	Array a_node;

	for (int j = 0; j < count_req; ++j) {
		int id;
		std::string type;
		std::string name;
		std::tie(id, type, name) = rh.GetRequestByNumber(j);
		if (type == "Stop"s) {
			a_node.push_back(NodeAnswerBusesByStop(rh, id, name));
		}
		else if (type == "Bus"s) {
			a_node.push_back(NodeAnswerBusStatistics(rh, id, name));
		}
		else {
			a_node.push_back(SvgAnswerRenderMap(rh, id));
		}
	}
		
	json::Document doc(std::move(a_node));
	Print(doc, output);
}

Dict NodeAnswerBusesByStop(const RequestHandler& rh, const int id, const std::string& name) {
	
	Dict d_node;

	if (rh.GetStopBusByName(name) == nullptr) {
		d_node["request_id"] = id;
		d_node["error_message"] = "not found"s;
		return d_node;
	}

	Array a_node;
	const std::unordered_set<BusPtr>* ptr_buses = std::move(rh.GetBusesByStop(name));

	if (ptr_buses != nullptr) {
		vector<BusPtr> buses(ptr_buses->begin(), ptr_buses->end());
		std::sort(buses.begin(), buses.end(), [](BusPtr lhs, BusPtr rhs) {return lhs->name < rhs->name;});

		for (BusPtr ptrbus : buses) {
			a_node.push_back(ptrbus->name);
		}
	}

	d_node["buses"] = a_node;
	d_node["request_id"] = id;

	delete ptr_buses;

	return d_node;
}

Dict NodeAnswerBusStatistics(const RequestHandler& rh, const int id, const std::string& name) {
	std::optional<BusStat> busstat = rh.GetBusStat(name);
	Dict node;

	if (!busstat.has_value()) {
		node["request_id"] = id;
		node["error_message"] = "not found"s;
	}
	else {
		node["curvature"] = busstat.value().curvature;
		node["request_id"] = id;
		node["route_length"] = busstat.value().distance;
		node["stop_count"] = static_cast<int>(busstat.value().count_stopbus);
		node["unique_stop_count"] = static_cast<int>(busstat.value().uniq_stopbus);
	}

	return node;
}

Dict SvgAnswerRenderMap(const RequestHandler& rh, const int id) {
	Dict d_node;
	svg::Document doc = rh.RenderMap();
	std::ostringstream s_out;
	doc.Render(s_out);
	std::string print_str;
	print_str = s_out.str();

	d_node["map"] = print_str;
	d_node["request_id"] = id;

	return d_node;
}

void LoadRendererSettingFromJson(MapRenderer& mr, const json::Document& doc) {
	Dict top_dict = doc.GetRoot().AsMap();
	Dict render_settings = top_dict["render_settings"].AsMap();
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
	map_settings.underlayer_color = mr.GetColorFromJsonNote(render_settings["underlayer_color"]);
	map_settings.underlayer_width = render_settings["underlayer_width"].AsDouble();
	map_settings.color_palette = render_settings["color_palette"].AsArray();

	mr.SetMapSettings(map_settings);
}