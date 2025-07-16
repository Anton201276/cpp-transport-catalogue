#pragma once

#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <string_view>
#include <vector>
#include <optional>
#include <variant>
#include <utility>
#include <iomanip>

namespace svg {

    using namespace std::literals;

    struct Point {
        Point() = default;
        Point(double x, double y)
            : x(x)
            , y(y) {
        }
        double x = 0;
        double y = 0;
    };

    struct Rgb {
        Rgb() = default;
        Rgb(uint8_t red, uint8_t green, uint8_t blue)
            : red(red)
            , green(green)
            , blue(blue) {
        }
        uint8_t red = 0;
        uint8_t green = 0;
        uint8_t blue = 0;
    };

    struct Rgba {
        Rgba() = default;
        Rgba(uint8_t red, uint8_t green, uint8_t blue, double opacity)
            : red(red)
            , green(green)
            , blue(blue)
            , opacity(opacity) {
        }
        uint8_t red = 0;
        uint8_t green = 0;
        uint8_t blue = 0;
        double opacity = 1.0;
    };

    enum class StrokeLineCap {
        BUTT,
        ROUND,
        SQUARE,
    };

    enum class StrokeLineJoin {
        ARCS,
        BEVEL,
        MITER,
        MITER_CLIP,
        ROUND,
    };

    using Color = std::variant<std::monostate, std::string, Rgb, Rgba>;
    inline const Color NoneColor{ "none" };

    inline std::ostream& operator<<(std::ostream& stream, const StrokeLineCap& linecap) {
        switch (linecap)
        {
        case StrokeLineCap::BUTT:
            stream << "butt"sv;
            break;
        case StrokeLineCap::ROUND:
            stream << "round"sv;
            break;
        case StrokeLineCap::SQUARE:
            stream << "square"sv;
            break;
        default:
            stream << ""sv;
        }
        return stream;
    }

    inline std::ostream& operator<<(std::ostream& stream, const StrokeLineJoin& linejoin) {
        switch (linejoin)
        {
        case StrokeLineJoin::ARCS:
            stream << "arcs"sv;
            break;
        case StrokeLineJoin::BEVEL:
            stream << "bevel"sv;
            break;
        case StrokeLineJoin::MITER:
            stream << "miter"sv;
            break;
        case StrokeLineJoin::MITER_CLIP:
            stream << "miter-clip"sv;
            break;
        case StrokeLineJoin::ROUND:
            stream << "round"sv;
            break;
        default:
            stream << ""sv;
        }
        return stream;
    }

    //inline uint8_t Lerp(uint8_t from, uint8_t to, double t) {
    //    return static_cast<uint8_t>(std::round((to - from) * t + from));
    //}

    struct ColorPrinter {
        std::ostream& out;
        void operator()(std::monostate) const {
            out << "none"sv;
        }
        void operator()(std::string color) const {
            out << color;
        }
        void operator()(Rgb color) const {
            const std::string r = std::to_string(color.red);
            const std::string g = std::to_string(color.green);
            const std::string b = std::to_string(color.blue);

            std::string fill_color = "rgb("s + r + ","s + g + ","s + b + ")"s;
            out << fill_color;
        }

        void operator()(Rgba color) const {
            const std::string r = std::to_string(color.red);
            const std::string g = std::to_string(color.green);
            const std::string b = std::to_string(color.blue);
            const std::string o = std::to_string(color.opacity);

            //std::string fill_color = "rgba("s + r + ","s + g + ","s + b + ","s + color.opacity +")"s;
            out << "rgba("s << r << ","s << g << ","s << b << ","s << color.opacity << ")"s;
            
        }
    };

    inline std::ostream& operator<<(std::ostream& stream, const Color& color) {
        std::visit(ColorPrinter{ stream }, color);
        return stream;
    }

    /*
     * Вспомогательная структура, хранящая контекст для вывода SVG-документа с отступами.
     * Хранит ссылку на поток вывода, текущее значение и шаг отступа при выводе элемента
     */
    struct RenderContext {
        RenderContext(std::ostream& out)
            : out(out) {
        }

        RenderContext(std::ostream& out, int indent_step, int indent = 0)
            : out(out)
            , indent_step(indent_step)
            , indent(indent) {
        }

        RenderContext Indented() const {
            return { out, indent_step, indent + indent_step };
        }

        void RenderIndent() const {
            for (int i = 0; i < indent; ++i) {
                out.put(' ');
            }
        }

        std::ostream& out;
        int indent_step = 0;
        int indent = 0;
    };

    /*
     * Абстрактный базовый класс Object служит для унифицированного хранения
     * конкретных тегов SVG-документа
     * Реализует паттерн "Шаблонный метод" для вывода содержимого тега
     */
    class Object {
    public:
        void Render(const RenderContext& context) const;

        virtual ~Object() = default;

    private:
        virtual void RenderObject(const RenderContext& context) const = 0;
    };

    template <typename Owner>
    class PathProps {
    public:
        Owner& SetFillColor(Color color) {
            fill_color_ = std::move(color);
            return AsOwner();
        }
        Owner& SetStrokeColor(Color color) {
            stroke_color_ = std::move(color);
            return AsOwner();
        }

        Owner& SetStrokeWidth(double width) {
            width_ = std::move(width);
            return AsOwner();
        }

        Owner& SetStrokeLineCap(StrokeLineCap line_cap) {
            line_cap_ = std::move(line_cap);
            return AsOwner();
        }

        Owner& SetStrokeLineJoin(StrokeLineJoin line_join) {
            line_join_ = std::move(line_join);
            return AsOwner();
        }

    protected:
        ~PathProps() = default;

        void RenderAttrs(std::ostream& out) const {
            using namespace std::literals;

            if (fill_color_) {
                out << " fill=\""sv << *fill_color_ << "\""sv;
            }
            if (stroke_color_) {
                out << " stroke=\""sv << *stroke_color_ << "\""sv;
            }
            if (width_) {
                out << " stroke-width=\""sv << *width_ << "\""sv;
            }
            if (line_cap_) {
                out << " stroke-linecap=\""sv << *line_cap_ << "\""sv;
            }
            if (line_join_) {
                out << " stroke-linejoin=\""sv << *line_join_ << "\""sv;
            }
        }

    private:
        Owner& AsOwner() {
            return static_cast<Owner&>(*this);
        }

        std::optional<Color> fill_color_;
        std::optional<Color> stroke_color_;
        std::optional<double> width_;
        std::optional<StrokeLineCap> line_cap_;
        std::optional<StrokeLineJoin> line_join_;
    };

    /*
     * Класс Circle моделирует элемент <circle> для отображения круга
     * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/circle
     */
    class Circle final : public Object, public PathProps<Circle> {
    public:
        Circle& SetCenter(Point center);
        Circle& SetRadius(double radius);

    private:
        void RenderObject(const RenderContext& context) const override;

        Point center_;
        double radius_ = 1.0;
    };

    /*
     * Класс Polyline моделирует элемент <polyline> для отображения ломаных линий
     * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/polyline
     */
    class Polyline final : public Object, public PathProps<Polyline> {
    public:
        // Добавляет очередную вершину к ломаной линии
        Polyline& AddPoint(Point point);

        /*
         * Прочие методы и данные, необходимые для реализации элемента <polyline>
         */
    private:
        void RenderObject(const RenderContext& context) const override;

        std::vector<Point> polyline_node_;
    };

    /*
     * Класс Text моделирует элемент <text> для отображения текста
     * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/text
     */
    class Text final : public Object, public PathProps<Text> {
    public:
        Text() = default;

        // Задаёт координаты опорной точки (атрибуты x и y)
        Text& SetPosition(Point pos);

        // Задаёт смещение относительно опорной точки (атрибуты dx, dy)
        Text& SetOffset(Point offset);

        // Задаёт размеры шрифта (атрибут font-size)
        Text& SetFontSize(uint32_t size);

        // Задаёт название шрифта (атрибут font-family)
        Text& SetFontFamily(std::string font_family);

        // Задаёт толщину шрифта (атрибут font-weight)
        Text& SetFontWeight(std::string font_weight);

        // Задаёт текстовое содержимое объекта (отображается внутри тега text)
        Text& SetData(std::string data);

        // Прочие данные и методы, необходимые для реализации элемента <text>
    private:
        void RenderObject(const RenderContext& context) const override;

        Point pos_{ 0.0,0.0 };
        Point offset_{ 0.0,0.0 };
        uint32_t size_{ 1 };
        std::string font_family_{};
        std::string font_weight_{};
        std::string data_{};
    };

    class ObjectContainer {
    public:
        template <typename Obj>
        void Add(Obj obj) {
            AddPtr(std::move(std::make_unique<Obj>(obj)));
        }

        virtual void AddPtr(std::unique_ptr<Object>&& obj) = 0;

    protected:
        ~ObjectContainer() = default;
    };
    
    class Document : public ObjectContainer {
    public:
        Document() = default;
        
        void AddPtr(std::unique_ptr<Object>&& obj) override;

        // Выводит в ostream svg-представление документа
        void Render(std::ostream& out) const;

        // Прочие методы и данные, необходимые для реализации класса Document
    private:
        std::vector<std::unique_ptr<Object>> objects_;
    };

    class Drawable {
    public:
        virtual void Draw(ObjectContainer& obj) const = 0;
        virtual ~Drawable() = default;
    };

}  // namespace svg