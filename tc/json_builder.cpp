#include "json_builder.h"

namespace json {

  KeyItemContext Builder::Key(string k) {
	if (!root_.IsNull() && nodes_stack_.empty()) {
	  throw logic_error("Key key after end");
	}
	if (nodes_stack_.empty()) {
	  throw logic_error("Key no map");
	}
	if (!nodes_stack_.top()->IsDict()) {
	  throw logic_error("Key top stack no map");
	}
	if (key_.has_value()) {
	  throw logic_error("Key error double key");
	}
	key_ = std::move(k);
	return KeyItemContext {*this};
  }

  Builder& Builder::Value(Node v) {
	if (!root_.IsNull() && nodes_stack_.empty()) {
	  throw logic_error("Value error !null && empty stack");
	}
	if (!(root_.IsNull() || key_.has_value() || nodes_stack_.top()->IsArray())) {
	  throw logic_error("Value error valie insert");
	}
	if (root_.IsNull()) {
	  root_ = std::move(v);
	  if (!is_first && (root_.IsDict() || root_.IsArray())) {
		nodes_stack_.push(&root_);
	  }
	  is_first = false;
	} else if (!nodes_stack_.empty() && nodes_stack_.top()->IsDict()) {
	  Dict& dict = const_cast<Dict&>(nodes_stack_.top()->AsDict());
	  auto [it_insert, result_insert]
		  = dict.emplace(std::move(key_.value()), std::move(v));
	  if (result_insert && (it_insert->second.IsDict() || it_insert->second.IsArray())) {
		nodes_stack_.push(&it_insert->second);
	  }
	  key_.reset();
	} else if (!nodes_stack_.empty() && nodes_stack_.top()->IsArray()) {
	  Array& array = const_cast<Array&>(nodes_stack_.top()->AsArray());
	  array.push_back(std::move(v));
	  if (array.back().IsDict() || array.back().IsArray()) {
		nodes_stack_.push(&array.back());
	  }
	}
	return *this;
  }

  Node Builder::Build() {
	if (root_.IsNull() || !nodes_stack_.empty()) {
	  throw logic_error("Build buld error, open structures or null root");
	}
	return root_;
  }

  Builder& Builder::EndArray() {
	if (!root_.IsNull() && nodes_stack_.empty()) {
	  throw logic_error("EndArray error close array");
	}
	if (nodes_stack_.empty() || !nodes_stack_.top()->IsArray()) {
	  throw logic_error("EndArray error close array");
	}
	nodes_stack_.pop();
	return *this;
  }

  ArrayItemContext Builder::StartArray() {
	is_first = false;
	Value(Node(Array {}));
	return ArrayItemContext {*this};
  }

  Builder& Builder::EndDict() {
	if (!root_.IsNull() && nodes_stack_.empty()) {
	  throw logic_error("EndDict error end dict root & ");
	}
	if (key_.has_value()) {
	  throw logic_error("EndDict error key no pair vflue");
	}
	if (nodes_stack_.empty() || !nodes_stack_.top()->IsDict()) {
	  throw logic_error("EndDict on top stack non dict");
	}
	nodes_stack_.pop();
	return *this;
  }

  DictItemContext Builder::StartDict() {
	is_first = false;
	Value(Node(Dict {}));
	return DictItemContext(*this);
  }

  // ******** KeyItemContext

  KeyItemContext::KeyItemContext(Builder& b) : ItemContext {b} {}

  ValueItemContext KeyItemContext::Value(Node v) {
	return ValueItemContext(b_.Value(v));
  }

  DictItemContext KeyItemContext::StartDict() {
	return DictItemContext(b_.StartDict());
  }

  ArrayItemContext KeyItemContext::StartArray() {
	return ArrayItemContext(b_.StartArray());
  }

  // *******DictItemContext

  DictItemContext::DictItemContext(Builder& b) : ItemContext {b} {}

  KeyItemContext DictItemContext::Key(string k) {
	return KeyItemContext(b_.Key(k));
  }

  Builder& DictItemContext::EndDict() {
	return b_.EndDict();
  }

  //********** ValueItemContext

  ValueItemContext::ValueItemContext(Builder& b) : ItemContext {b} {}

  KeyItemContext ValueItemContext::Key(string k) {
	return KeyItemContext(b_.Key(k));
  }

  Builder& ValueItemContext::EndDict() {
	return b_.EndDict();
  }

  //*********** ArrayItemContext

  ArrayItemContext::ArrayItemContext(Builder& b) : ItemContext {b} {}

  DictItemContext ArrayItemContext::StartDict() {
	return DictItemContext(b_.StartDict());
  }

  ArrayItemContext ArrayItemContext::StartArray() {
	return ArrayItemContext(b_.StartArray());
  }

  Builder& ArrayItemContext::EndArray() {
	return b_.EndArray();
  }

  ArrayItemContext ArrayItemContext::Value(Node v) {
	return ArrayItemContext(b_.Value(v));
  }

  // **************** BASE

  ItemContext::ItemContext(Builder& b) : b_ {b} {}

  ItemContext::~ItemContext() {}

}  // namespace json
