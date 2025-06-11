#include "json.h"

using namespace std;

namespace json {

    namespace {

        Node LoadNode(istream& input);

        Node LoadArray(istream& input) {
            Array result;

            for (char c; input >> c && c != ']';) {
                if (c != ',') {
                    input.putback(c);
                }
                result.push_back(LoadNode(input));
            }

            if (!input) throw ParsingError("error data format");

            return Node(move(result));
        }

        Node LoadString(std::istream& input) {
            using namespace std::literals;

            auto it = std::istreambuf_iterator<char>(input);
            auto end = std::istreambuf_iterator<char>();
            std::string s;
            while (true) {
                if (it == end) {
                    // Поток закончился до того, как встретили закрывающую кавычку?
                    throw ParsingError("String parsing error");
                }
                const char ch = *it;
                if (ch == '"') {
                    // Встретили закрывающую кавычку
                    ++it;
                    break;
                }
                else if (ch == '\\') {
                    // Встретили начало escape-последовательности
                    ++it;
                    if (it == end) {
                        // Поток завершился сразу после символа обратной косой черты
                        throw ParsingError("String parsing error");
                    }
                    const char escaped_char = *(it);
                    // Обрабатываем одну из последовательностей: \\, \n, \t, \r, \"
                    switch (escaped_char) {
                    case 'n':
                        s.push_back('\n');
                        break;
                    case 't':
                        s.push_back('\t');
                        break;
                    case 'r':
                        s.push_back('\r');
                        break;
                    case '"':
                        s.push_back('"');
                        break;
                    case '\\':
                        s.push_back('\\');
                        break;
                    default:
                        // Встретили неизвестную escape-последовательность
                        throw ParsingError("Unrecognized escape sequence \\"s + escaped_char);
                    }
                }
                else if (ch == '\n' || ch == '\r') {
                    // Строковый литерал внутри- JSON не может прерываться символами \r или \n
                    throw ParsingError("Unexpected end of line"s);
                }
                else {
                    // Просто считываем очередной символ и помещаем его в результирующую строку
                    s.push_back(ch);
                }
                ++it;
            }

            return s;
        }

        Node LoadDict(istream& input) {
            Dict result;

            for (char c; input >> c && c != '}';) {
                if (c == ',') {
                    input >> c;
                }

                string key = LoadString(input).AsString();
                input >> c;
                result.insert({ move(key), LoadNode(input) });
            }
            if (!input) throw ParsingError("error data format");

            return Node(move(result));
        }

        Node LoadNumber(std::istream& input) {
            using namespace std::literals;

            std::string parsed_num;

            // Считывает в parsed_num очередной символ из input
            auto read_char = [&parsed_num, &input] {
                parsed_num += static_cast<char>(input.get());
                if (!input) {
                    throw ParsingError("Failed to read number from stream"s);
                }
            };

            // Считывает одну или более цифр в parsed_num из input
            auto read_digits = [&input, read_char] {
                if (!std::isdigit(input.peek())) {
                    throw ParsingError("A digit is expected"s);
                }
                while (std::isdigit(input.peek())) {
                    read_char();
                }
            };

            if (input.peek() == '-') {
                read_char();
            }
            // Парсим целую часть числа
            if (input.peek() == '0') {
                read_char();
                // После 0 в JSON не могут идти другие цифры
            }
            else {
                read_digits();
            }

            bool is_int = true;
            // Парсим дробную часть числа
            if (input.peek() == '.') {
                read_char();
                read_digits();
                is_int = false;
            }

            // Парсим экспоненциальную часть числа
            if (int ch = input.peek(); ch == 'e' || ch == 'E') {
                read_char();
                if (ch = input.peek(); ch == '+' || ch == '-') {
                    read_char();
                }
                read_digits();
                is_int = false;
            }

            try {
                if (is_int) {
                    // Сначала пробуем преобразовать строку в int
                    try {
                        return std::stoi(parsed_num);
                    }
                    catch (...) {
                        // В случае неудачи, например, при переполнении,
                        // код ниже попробует преобразовать строку в double
                    }
                }
                return std::stod(parsed_num);
            }
            catch (...) {
                throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
            }
        }

        Node LoadLogicNull(istream& input) {
            string temp;

            while (isalpha(input.peek())) {
                temp += static_cast<char>(input.get());
            }

            if (temp == "false") {
                return Node(false);
            }
            else if (temp == "true") {
                return Node(true);
            }
            else if (temp == "null") {
                return Node(nullptr);
            }

            throw ParsingError(string("unexpected token: ") + temp);

            return Node();
        }

        Node LoadNode(istream& input) {
            char ch;
            input >> ch;

            if (isdigit(ch) || ch == '-') {
                input.putback(ch);
                return LoadNumber(input);
            }
            else if (ch == '\"') {
                return LoadString(input);
            }
            else if (ch == '{') {
                return LoadDict(input);
            }
            else if (ch == '[') {
                return LoadArray(input);
            }
            else {
                input.putback(ch);
                return LoadLogicNull(input);
            }

            return Node();
        }

    }  // namespace

    bool Node::IsInt() const {
        return std::holds_alternative<int>(value_) ? true : false;
    }

    bool Node::IsDouble() const {
        return std::holds_alternative<double>(value_) || IsInt() ? true : false;
    }

    bool Node::IsPureDouble() const {
        return std::holds_alternative<double>(value_) ? true : false;
    }

    bool Node::IsBool() const {
        return std::holds_alternative<bool>(value_) ? true : false;
    }

    bool Node::IsString() const {
        return std::holds_alternative<string>(value_) ? true : false;
    }

    bool Node::IsArray() const {
        return std::holds_alternative<Array>(value_) ? true : false;
    }

    bool Node::IsNull() const {
        return std::holds_alternative<std::nullptr_t>(value_) ? true : false;
    }

    bool Node::IsMap() const {
        return std::holds_alternative<Dict>(value_) ? true : false;
    }

    Node::Node(std::nullptr_t nll)
        :value_(nll) {
    }

    Node::Node(Array array)
        : value_(move(array)) {
    }

    Node::Node(Dict map)
        : value_(move(map)) {
    }

    Node::Node(int value)
        : value_(value) {
    }

    Node::Node(string value)
        : value_(move(value)) {
    }

    Node::Node(bool value)
        : value_(value) {
    }

    Node::Node(double value)
        : value_(move(value)) {
    }

    const Array& Node::AsArray() const {
        if (IsArray()) {
            return std::get<Array>(value_);
        }
        throw std::logic_error("Type error");
    }

    const Dict& Node::AsMap() const {
        if (IsMap()) {
            return std::get<Dict>(value_);
        }
        throw std::logic_error("Type error");
    }

    int Node::AsInt() const {
        if (IsInt()) {
            return std::get<int>(value_);
        }
        throw std::logic_error("Type error");
    }

    const string& Node::AsString() const {
        if (IsString()) {
            return std::get<string>(value_);
        }
        throw std::logic_error("Type error");
    }

    double Node::AsDouble() const {
        if (IsPureDouble()) {
            return std::get<double>(value_);
        }
        else if (IsInt()) {
            return static_cast<double>(get<int>(value_));
        }
        throw std::logic_error("Type error");
    }

    bool Node::AsBool() const {
        if (IsBool()) {
            return std::get<bool>(value_);
        }
        throw std::logic_error("Type error");
    }

    bool  Node::operator==(const Node& rhs) const {
        if (this->IsNull() && rhs.IsNull())
            return true;
        else if (this->IsArray() && rhs.IsArray()) {
            if (this->AsArray() == rhs.AsArray()) {
                return true;
            }
        }
        else if (this->IsInt() && rhs.IsInt()) {
            if (this->AsInt() == rhs.AsInt()) {
                return true;
            }
        }
        else if (this->IsPureDouble() && rhs.IsPureDouble()) {
            if (this->AsDouble() == rhs.AsDouble()) {
                return true;
            }
        }
        else if (this->IsBool() && rhs.IsBool()) {
            if (this->AsBool() == rhs.AsBool()) {
                return true;
            }
        }
        else if (this->IsString() && rhs.IsString()) {
            if (this->AsString() == rhs.AsString()) {
                return true;
            }
        }
        else if (this->IsMap() && rhs.IsMap()) {
            if (this->AsMap() == rhs.AsMap()) {
                return true;
            }
        }
        else {
            return false;
        }
        return false;
    }

    bool  Node::operator!=(const Node& rhs) const {
        return !(*this == rhs);
    }

    Document::Document(Node root)
        : root_(move(root)) {
    }

    const Node& Document::GetRoot() const {
        return root_;
    }

    bool Document::operator==(const Document& rhs) const {
        return this->GetRoot() == rhs.GetRoot();
    }

    bool Document::operator!=(const Document& rhs) const {
        return !(this->GetRoot() == rhs.GetRoot());
    }

    Document Load(istream& input) {
        return Document{ LoadNode(input) };
    }

    struct PrintContext {
        std::ostream& out;
        int indent_step = 2;
        int indent = 0;

        void PrintIndent() const {
            for (int i = 0; i < indent; ++i) {
                out.put(' ');
            }
        }

        // Возвращает новый контекст вывода с увеличенным смещением
        PrintContext Indented() const {
            return { out, indent_step, indent_step + indent };
        }
    };

    void PrintNode(const Node& value, const PrintContext& context);

    template <typename Value>
    void PrintValue(const Value& value, const PrintContext& context) {
        context.out << value;
    }

    void PrintString(const std::string& value, std::ostream& out) {
        out << '\"';
        for (char ch : value) {
            switch (ch) {
            case '\n':
                out << "\\n";
                break;
            case '\t':
                out << "\\t";
                break;
            case '\r':
                out << "\\r";
                break;
            case '\\':
                out << "\\\\";
                break;
            case '\"':
                out << "\\\"";
                break;
            default:
                out << ch;
            }
        }
        out << '\"';
    }

    template <>
    void PrintValue<std::string>(const std::string& value, const PrintContext& context) {
        PrintString(value, context.out);
    }

    template <>
    void PrintValue<std::nullptr_t>(const std::nullptr_t&, const PrintContext& context) {
        context.out << "null"sv;
    }

    template <>
    void PrintValue<bool>(const bool& value, const PrintContext& context) {
        context.out << (value ? "true"sv : "false"sv);
    }

    template <>
    void PrintValue<Array>(const Array& nodes, const PrintContext& context) {
        context.out << "[\n"s;
        context.PrintIndent();
        auto context_next = context.Indented();
        for (size_t i = 0; i < nodes.size(); i++) {
            if (i > 0) {
                context.out << ",\n";
            }
            context_next.PrintIndent();
            PrintNode(nodes[i], context_next);
        }
        context.out << "\n"s;
        context.PrintIndent();
        context.out << ']';
    }

    template <>
    void PrintValue<Dict>(const Dict& nodes, const PrintContext& context) {
        context.PrintIndent();
        context.out << "{\n"s;
        bool first = true;
        auto context_next = context.Indented();
        for (const auto& [key, item] : nodes) {
            if (!first) {
                context.out << ",\n"s;
            }
            context_next.PrintIndent();
            context.out << '\"' << key << "\": ";
            PrintNode(item, context_next);
            first = false;
        }
        context.out << "\n";
        context.PrintIndent();
        context.out << "}"s;
    }

    void PrintNode(const Node& node, const PrintContext& context) {
        std::visit(
            [&context](const auto& value) {
                PrintValue(value, context);
            },
            node.GetValue());
    }

    void Print(const Document& doc, ostream& out) {
        PrintNode(doc.GetRoot(), PrintContext{ out });
    }

    Document LoadJSON(const std::string& s) {
        std::istringstream strm(s);
        return json::Load(strm);
    }
}  // namespace json
