#pragma once

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <optional>
#include <vector>
#include <set>

#include "json.h"
#include "svg.h"
#include "geo.h"
#include "domain.h"


using namespace svg;
using namespace json;

namespace renderer {

	inline const double EPSILON = 1e-6;
	inline bool IsZero(double value) {
		return std::abs(value) < EPSILON;
	}

	class SphereProjector {
	public:
		// points_begin и points_end задают начало и конец интервала элементов geo::Coordinates
		template <typename PointInputIt>
		SphereProjector(PointInputIt points_begin, PointInputIt points_end,
			double max_width, double max_height, double padding)
			: padding_(padding) //
		{
			// Если точки поверхности сферы не заданы, вычислять нечего
			if (points_begin == points_end) {
				return;
			}

			// Находим точки с минимальной и максимальной долготой
			const auto [left_it, right_it] = std::minmax_element(
				points_begin, points_end,
				[](auto lhs, auto rhs) { return lhs.lng < rhs.lng; });
			min_lon_ = left_it->lng;
			const double max_lon = right_it->lng;

			// Находим точки с минимальной и максимальной широтой
			const auto [bottom_it, top_it] = std::minmax_element(
				points_begin, points_end,
				[](auto lhs, auto rhs) { return lhs.lat < rhs.lat; });
			const double min_lat = bottom_it->lat;
			max_lat_ = top_it->lat;

			// Вычисляем коэффициент масштабирования вдоль координаты x
			std::optional<double> width_zoom;
			if (!IsZero(max_lon - min_lon_)) {
				width_zoom = (max_width - 2 * padding) / (max_lon - min_lon_);
			}

			// Вычисляем коэффициент масштабирования вдоль координаты y
			std::optional<double> height_zoom;
			if (!IsZero(max_lat_ - min_lat)) {
				height_zoom = (max_height - 2 * padding) / (max_lat_ - min_lat);
			}

			if (width_zoom && height_zoom) {
				// Коэффициенты масштабирования по ширине и высоте ненулевые,
				// берём минимальный из них
				zoom_coeff_ = std::min(*width_zoom, *height_zoom);
			}
			else if (width_zoom) {
				// Коэффициент масштабирования по ширине ненулевой, используем его
				zoom_coeff_ = *width_zoom;
			}
			else if (height_zoom) {
				// Коэффициент масштабирования по высоте ненулевой, используем его
				zoom_coeff_ = *height_zoom;
			}
		}

		// Проецирует широту и долготу в координаты внутри SVG-изображения
		svg::Point operator()(geo::Coordinates coords) const {
			return {
				(coords.lng - min_lon_) * zoom_coeff_ + padding_,
				(max_lat_ - coords.lat) * zoom_coeff_ + padding_
			};
		}

	private:
		double padding_;
		double min_lon_ = 0;
		double max_lat_ = 0;
		double zoom_coeff_ = 0;
	};


    struct MapSettings {
        double wight = 600;
        double height = 400;
        double padding = 50;
        double stop_radius = 5;
        double line_width = 14;
        int bus_label_font_size = 20;
        double bus_label_offset[2] = { 7,15 };
        int stop_label_font_size = 20;
        double stop_label_offset[2] = { 7,-3 };
        svg::Color underlayer_color = Rgba{ 255, 255, 255, 0.85 };
        double underlayer_width = 3;
        Array color_palette = { Node("green"s), Node({Node(255), Node(160), Node(0)}), Node("red"s) };
    };

    

    class MapRenderer {
    public:
        MapRenderer() = default;
        MapRenderer(MapSettings map_settings) : map_settings_(map_settings) {};

        void SetMapSettings(MapSettings map_settings);
        svg::Color GetRgbaColorFromJson(double ary_rgba[4]) const;
        svg::Color GetRgbColorFromJson(int ary_rgba[3]) const;
        svg::Color GetColorFromJsonNote(json::Node node) const;
        svg::Color GetColorFromJsonArray(Array nodes, int id) const;
        
        svg::Document MapRenderBusTrip(const std::vector<BusPtr>& buses, const vector<StopPtr>& stops) const;
        void MapRenderBusPolyline(svg::Document& doc, const std::vector<BusPtr>& buses, const SphereProjector& sph_proj) const;
		void MapRenderBusNameText(svg::Document& doc, const std::vector<BusPtr>& buses, const SphereProjector& sph_proj) const;
		void FillSettingsBusNameText(svg::Document& doc, const std::string& name, const svg::Point& point, const Color& color) const;
		void MapRenderStopCircleName(svg::Document& doc, const vector<StopPtr>& stops, const SphereProjector& sph_proj) const;
		void FillSettingsStopNameText(svg::Document& doc, const std::string& name, const svg::Point& point, const Color& color) const;

    private:
        MapSettings map_settings_;
    };
}