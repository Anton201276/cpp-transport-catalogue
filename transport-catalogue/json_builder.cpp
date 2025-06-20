#include "json_builder.h"

namespace json {

	using namespace std::literals;

	Builder::Builder()
		: root_()
		, nodes_stack_{ &root_ }
	{}

	Node Builder::Build() {
		if (!nodes_stack_.empty()) {
			throw std::logic_error("Attempt to build JSON which isn't finalized"s);
		}
		return std::move(root_);
	}

	Node::Value& Builder::GetCurrentValue() {

		if (nodes_stack_.empty()) {
			throw std::logic_error("Error - JSON is not finish"s);
		}
		return nodes_stack_.back()->GetValue();
	}

	Builder::DictItemContext Builder::StartDict() {

		Node::Value& host_value = GetCurrentValue();

		if (std::holds_alternative<Array>(host_value)) {
			Node& node = std::get<Array>(host_value).emplace_back(std::move(Dict{}));
			nodes_stack_.push_back(&node);
		}
		else if (std::holds_alternative<std::nullptr_t>(host_value)) {
			host_value = Dict{};
		}
		else {
			throw std::logic_error("Can not create new object"s);
		}
		return DictItemContext{ *this };
	}
	
	Builder::DictValueContext Builder::Key(std::string key) {

		Node::Value& host_value = GetCurrentValue();

		if (std::holds_alternative<Dict>(host_value)) {
			auto& host_value = GetCurrentValue();
			nodes_stack_.push_back(&std::get<Dict>(host_value)[std::move(key)]);
		}
		else {
			throw std::logic_error("Dict was not expected here"s);
		}
		return DictValueContext{ *this };
	}

	Builder::BaseContext Builder::EndDict() {

		Node::Value& host_value = GetCurrentValue();

		if (std::holds_alternative<Dict>(host_value)) {
			nodes_stack_.pop_back();
			return *this;
		}
		else {
			throw std::logic_error("EndDict was not expected here"s);
		}
	}
	
	Builder::ArrayItemContext Builder::StartArray() {

		Node::Value& host_value = GetCurrentValue();

		if (std::holds_alternative<Array>(host_value)) {
			Node& node = std::get<Array>(host_value).emplace_back(std::move(Array{}));
			nodes_stack_.push_back(&node);
		}
		else if (std::holds_alternative<std::nullptr_t>(host_value)) {
			host_value = Array{};
		}
		else {
			throw std::logic_error("Can not create new object"s);
		}
		return BaseContext{ *this };
	}

	Builder::BaseContext Builder::EndArray() {

		Node::Value& host_value = GetCurrentValue();
		
		if (std::holds_alternative<Array>(host_value)) {
			nodes_stack_.pop_back();
			return BaseContext{ *this };
		}
		else {
			throw std::logic_error("EndArray was not expected here"s);
		}
	}

	Builder::BaseContext Builder::Value(Node::Value value) {

		Node::Value& host_value = GetCurrentValue();

		if (std::holds_alternative<Array>(host_value)) {
			std::get<Array>(host_value).emplace_back(std::move(value));
		}
		else if (std::holds_alternative<std::nullptr_t>(host_value)) {
			host_value = std::move(value);
			nodes_stack_.pop_back();
		}
		else {
			throw std::logic_error("Input Value was not expected here"s);
		}
		return *this;
	}
}

