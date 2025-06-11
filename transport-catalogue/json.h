#pragma once

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <variant>
#include <stdexcept>
#include <sstream>


namespace json {

    class Node;
    using Dict = std::map<std::string, Node>;
    using Array = std::vector<Node>;
    using Value = std::variant<std::nullptr_t, Array, Dict, bool, int, double, std::string>;


    class ParsingError : public std::runtime_error {
    public:
        using runtime_error::runtime_error;
    };

    class Node {
    public:

        Node() = default;
        Node(std::nullptr_t nll);
        Node(Array array);
        Node(Dict map);
        Node(bool value);
        Node(int value);
        Node(double value);
        Node(std::string value);

        bool IsInt() const;
        bool IsDouble() const;
        bool IsArray() const;
        bool IsPureDouble() const;
        bool IsBool() const;
        bool IsString() const;
        bool IsNull() const;
        bool IsMap() const;

        const Array& AsArray() const;
        const Dict& AsMap() const;
        bool AsBool() const;
        int AsInt() const;
        double AsDouble() const;
        const std::string& AsString() const;

        bool operator==(const Node& rhs) const;
        bool operator!=(const Node& rhs) const;

        const Value& GetValue() const {
            return value_;
        }

    private:
        Value value_ = nullptr;
    };

    class Document {
    public:
        explicit Document(Node root);

        const Node& GetRoot() const;

        bool operator==(const Document& rhs) const;
        bool operator!=(const Document& rhs) const;

    private:
        Node root_;
    };

    Document LoadJSON(const std::string& s);

    Document Load(std::istream& input);

    void Print(const Document& doc, std::ostream& output);

}  // namespace json