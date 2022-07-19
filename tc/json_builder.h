#include <optional>
#include <stack>
#include <string>

#include "json.h"

namespace json {

  using namespace std;

  class KeyItemContext;
  class DictItemContext;
  class ArrayItemContext;

  class Builder {
  public:
	KeyItemContext Key(string k);
	Builder& Value(Node v);
	DictItemContext StartDict();
	Builder& EndDict();
	ArrayItemContext StartArray();
	Builder& EndArray();
	Node Build();

  private:
	std::stack<Node*> nodes_stack_;
	Node root_ = nullptr;
	std::optional<std::string> key_;
	bool is_first = true;
  };

  class ItemContext {
  public:
	ItemContext(Builder& b);
	virtual ~ItemContext();

  protected:
	Builder& b_;
  };

  class ValueItemContext : public ItemContext {
  public:
	explicit ValueItemContext(Builder& b);
	KeyItemContext Key(string k);
	Builder& EndDict();
  };

  class KeyItemContext : public ItemContext {
  public:
	explicit KeyItemContext(Builder& b);
	ValueItemContext Value(Node v);
	DictItemContext StartDict();
	ArrayItemContext StartArray();
  };

  class DictItemContext : public ItemContext {
  public:
	explicit DictItemContext(Builder& b);
	KeyItemContext Key(string k);
	Builder& EndDict();
  };

  class ArrayItemContext : public ItemContext {
  public:
	explicit ArrayItemContext(Builder& b);
	DictItemContext StartDict();
	ArrayItemContext StartArray();
	Builder& EndArray();
	ArrayItemContext Value(Node v);
  };

}  // namespace json
