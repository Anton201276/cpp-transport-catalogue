#include "svg.h"

namespace svg {

    using namespace std::literals;

    void Object::Render(const RenderContext& context) const {
        context.RenderIndent();

        // Делегируем вывод тега своим подклассам
        RenderObject(context);

        context.out << std::endl;
    }

    // ---------- Circle ------------------

    Circle& Circle::SetCenter(Point center) {
        center_ = center;
        return *this;
    }

    Circle& Circle::SetRadius(double radius) {
        radius_ = radius;
        return *this;
    }

    void Circle::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
        out << "r=\""sv << radius_ << "\""sv;
        //out << "/>"sv;
        RenderAttrs(context.out);
        out << "/>"sv;
    }

    Polyline& Polyline::AddPoint(Point point) {
        polyline_node_.push_back(std::move(point));
        return *this;
    }

    void Polyline::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        if (polyline_node_.empty()) {
            out << "<polyline points=\"\" />"sv;
            return;
        }

        out << "<polyline points=\""sv;
        for (size_t j = 0; j < polyline_node_.size() - 1; ++j) {
            out << polyline_node_[j].x << ","sv << polyline_node_[j].y << " "sv;  
        }
        out << polyline_node_[polyline_node_.size() - 1].x << ","sv << polyline_node_[polyline_node_.size() - 1].y << "\""sv;
        RenderAttrs(context.out);
        out << "/>"sv;
    }

    Text& Text::SetPosition(Point pos) {
        pos_ = pos;
        return *this;
    }

    Text& Text::SetOffset(Point offset) {
        offset_ = offset;
        return *this;
    }

    Text& Text::SetFontSize(uint32_t size) {
        size_ = size;
        return *this;
    }

    Text& Text::SetFontFamily(std::string font_family) {
        font_family_ = font_family;
        return *this;
    }

    Text& Text::SetFontWeight(std::string font_weight) {
        font_weight_ = font_weight;
        return *this;
    }

    Text& Text::SetData(std::string data) {
        data_ = data;
        return *this;
    }

    void Text::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        size_t pos_next = 0;
        size_t pos_prev = 0;
        std::string tmp_data{};
        bool cond = false;
        do {
            pos_next = data_.find_first_of("\"<>'&", pos_prev);
            cond = pos_next != data_.npos ? true : false;
            if (cond) {
                if (data_[pos_next] == '"') {
                    tmp_data += data_.substr(pos_prev, pos_next - pos_prev + 1) + "&quot;"s;
                }
                else if (data_[pos_next] == '<') {
                    tmp_data += data_.substr(pos_prev, pos_next - pos_prev + 1) + "&lt;"s;
                }
                else if (data_[pos_next] == '>') {
                    tmp_data += data_.substr(pos_prev, pos_next - pos_prev + 1) + "&gt;"s;
                }
                else if (data_[pos_next] == '\'') {
                    tmp_data += data_.substr(pos_prev, pos_next - pos_prev + 1) + "&apos;"s;
                }
                else {
                    tmp_data += data_.substr(pos_prev, pos_next - pos_prev + 1) + "&amp;"s;
                }
                pos_prev = pos_next + 1;
            }
            else {
                tmp_data += data_.substr(pos_prev, data_.size() - pos_prev);
            }
        } while (cond);
        
        out << "<text"sv;
        RenderAttrs(context.out);
        out << " x=\""sv << pos_.x << "\" y=\"" << pos_.y << "\" dx=\""sv << offset_.x
            << "\" dy=\""sv << offset_.y << "\" ";
        out << "font-size=\""sv << size_ << "\""sv;
        if (!font_family_.empty()) out << " font-family=\""sv << font_family_ << "\""sv;
        if (!font_weight_.empty()) out << " font-weight=\""sv << font_weight_ << "\""sv;
        out << ">"sv << tmp_data << "</text>"sv;
        
    }

    void Document::AddPtr(std::unique_ptr<Object>&& obj) {
        objects_.emplace_back(std::move(obj));
    }

    void Document::Render(std::ostream& out) const {
        out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>" << "\n";
        out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"sv << "\n";
        if (!objects_.empty()) {
            for (size_t j = 0; j < objects_.size(); ++j) {
                objects_[j]->Render({ out,2,2 });
            }
        }
        out << "</svg>";
    }

}  // namespace svg
