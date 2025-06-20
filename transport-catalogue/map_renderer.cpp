#include "map_renderer.h"
#include <iostream>

namespace renderer {

	void MapRenderer::SetMapSettings(MapSettings map_settings) {
		map_settings_ = std::move(map_settings);
	}

	svg::Color MapRenderer::GetRgbaColorFromJson(double ary_rgba[4]) const {
		uint8_t r = static_cast<uint8_t>(ary_rgba[0]);
		uint8_t g = static_cast<uint8_t>(ary_rgba[1]);
		uint8_t b = static_cast<uint8_t>(ary_rgba[2]);
		double a = ary_rgba[3];
		svg::Rgba stc_rgba{ r,g,b,a };
		return stc_rgba;
	}

	svg::Color MapRenderer::GetRgbColorFromJson(int ary_rgb[3]) const {
		uint8_t r = static_cast<uint8_t>(ary_rgb[0]);
		uint8_t g = static_cast<uint8_t>(ary_rgb[1]);
		uint8_t b = static_cast<uint8_t>(ary_rgb[2]);
		svg::Rgb stc_rgb{ r,g,b};
		return stc_rgb;
	}

	svg::Color MapRenderer::GetColorFromJsonNode(json::Node node) const {
		if (node.IsString()) {
			return node.AsString();
		}
		if (node.IsArray()) {
			Array ar_node = node.AsArray();
			if (ar_node.size() == 3) {
				int ary_rgb[3];
				for (int j = 0; j < 3; ++j) {
					ary_rgb[j] = ar_node[j].AsInt();
				}
				return GetRgbColorFromJson(ary_rgb);
			}
			else {
				double ary_rgba[4];
				for (int j = 0; j < 4; ++j) {
					ary_rgba[j] = ar_node[j].AsDouble();
				}
				return GetRgbaColorFromJson(ary_rgba);
			}
		}
		return NoneColor;
	}

	svg::Color MapRenderer::GetColorFromJsonArray(Array nodes, int id) const {
		return GetColorFromJsonNode(nodes[id]);
	}

	svg::Document MapRenderer::MapRenderBusTrip(const std::vector<BusPtr>& buses, const vector<StopPtr>& stops) const {
		
		svg::Document doc;
		std::vector<Coordinates> stop_coordinates;

		for (auto& stop : stops) {
			stop_coordinates.push_back(stop->coordinates);
		}

		const SphereProjector proj{ stop_coordinates.begin(), stop_coordinates.end(),
				map_settings_.wight,map_settings_.height,map_settings_.padding };

		MapRenderBusPolyline(doc, buses, proj);
		MapRenderBusNameText(doc, buses, proj);
		MapRenderStopCircleName(doc, stops, proj);

		return doc;
	}

	void MapRenderer::MapRenderBusPolyline(svg::Document& doc, const std::vector<BusPtr>& buses, const SphereProjector& sph_proj) const {
		int color_idx = 0;
		int color_idx_max = map_settings_.color_palette.size();
		for (auto& bus : buses) {
			Polyline pl;

			pl.SetFillColor("none");
			Color fromArray = map_settings_.color_palette[color_idx];
			pl.SetStrokeColor(fromArray);
			pl.SetStrokeLineCap(StrokeLineCap::ROUND);
			pl.SetStrokeLineJoin(StrokeLineJoin::ROUND);
			pl.SetStrokeWidth(map_settings_.line_width);

			for (const auto& stop : bus->busstop_info) {
				const svg::Point screen_coord = sph_proj(stop->coordinates);
				pl.AddPoint(screen_coord);
			}
			doc.Add(pl);
			++color_idx;
			if (color_idx > color_idx_max - 1) {
				color_idx = 0;
			}
		}
	}

	void MapRenderer::MapRenderBusNameText(svg::Document& doc, const std::vector<BusPtr>& buses, const SphereProjector& sph_proj) const {
		int color_idx = 0;
		int color_idx_max = map_settings_.color_palette.size();
		for (auto& bus : buses) {
			Text txt_m1;
			Text txt_u1;
			const svg::Point screen_coord = sph_proj(bus->busstop_info.front()->coordinates);
			Color frompalette = map_settings_.color_palette[color_idx];

			FillSettingsBusNameText(doc, bus->name, screen_coord, frompalette);

			int mid = bus->busstop_info.size() / 2;
			if (bus->type == NotRoundTrip && bus->busstop_info[mid] != bus->busstop_info.back()) {
				const svg::Point screen_coord = sph_proj(bus->busstop_info[mid]->coordinates);
				FillSettingsBusNameText(doc, bus->name, screen_coord, frompalette);
			}

			++color_idx;
			if (color_idx > color_idx_max - 1) {
				color_idx = 0;
			}
		}
	}

	void MapRenderer::FillSettingsBusNameText(svg::Document& doc, const std::string& name, const svg::Point& point, const Color& color) const {
		Text txt_m1;
		Text txt_u1;
		txt_u1.SetPosition(point);
		txt_u1.SetOffset({ map_settings_.bus_label_offset[0],map_settings_.bus_label_offset[1] });
		txt_u1.SetFontSize(map_settings_.bus_label_font_size);
		txt_u1.SetFontFamily("Verdana");
		txt_u1.SetFontWeight("bold");
		txt_u1.SetData(name);
		txt_u1.SetFillColor(map_settings_.underlayer_color);
		txt_u1.SetStrokeColor(map_settings_.underlayer_color);
		txt_u1.SetStrokeWidth(map_settings_.underlayer_width);
		txt_u1.SetStrokeLineCap(StrokeLineCap::ROUND);
		txt_u1.SetStrokeLineJoin(StrokeLineJoin::ROUND);
		doc.Add(txt_u1);

		txt_m1.SetPosition(point);
		txt_m1.SetOffset({ map_settings_.bus_label_offset[0],map_settings_.bus_label_offset[1] });
		txt_m1.SetFontSize(map_settings_.bus_label_font_size);
		txt_m1.SetFontFamily("Verdana");
		txt_m1.SetFontWeight("bold");
		txt_m1.SetData(name);
		txt_m1.SetFillColor(color);
		doc.Add(txt_m1);
	}

	void MapRenderer::MapRenderStopCircleName(svg::Document& doc, const vector<StopPtr>& stops, const SphereProjector& sph_proj) const {
		for (auto& stop : stops) {
			Circle cr;
			const svg::Point screen_coord = sph_proj(stop->coordinates);
			cr.SetCenter(screen_coord);
			cr.SetRadius(map_settings_.stop_radius);
			cr.SetFillColor("white");
			doc.Add(cr);
		}

		for (auto& stop : stops) {
			Text txt;
			const svg::Point screen_coord = sph_proj(stop->coordinates);
			FillSettingsStopNameText(doc, stop->name, screen_coord, "black");
		}
	}

	void MapRenderer::FillSettingsStopNameText(svg::Document& doc, const std::string& name, const svg::Point& point, const Color& color) const {
		Text txt_m1;
		Text txt_u1;
		txt_u1.SetPosition(point);
		txt_u1.SetOffset({ map_settings_.stop_label_offset[0],map_settings_.stop_label_offset[1] });
		txt_u1.SetFontSize(map_settings_.stop_label_font_size);
		txt_u1.SetFontFamily("Verdana");
		txt_u1.SetData(name);
		txt_u1.SetFillColor(map_settings_.underlayer_color);
		txt_u1.SetStrokeColor(map_settings_.underlayer_color);
		txt_u1.SetStrokeWidth(map_settings_.underlayer_width);
		txt_u1.SetStrokeLineCap(StrokeLineCap::ROUND);
		txt_u1.SetStrokeLineJoin(StrokeLineJoin::ROUND);
		doc.Add(txt_u1);

		txt_m1.SetPosition(point);
		txt_m1.SetOffset({ map_settings_.stop_label_offset[0],map_settings_.stop_label_offset[1] });
		txt_m1.SetFontSize(map_settings_.stop_label_font_size);
		txt_m1.SetFontFamily("Verdana");
		txt_m1.SetData(name);
		txt_m1.SetFillColor(color);
		doc.Add(txt_m1);
	}
}