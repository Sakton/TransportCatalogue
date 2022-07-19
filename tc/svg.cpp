#include "svg.h"

namespace svg {

  using namespace std::literals;

  std::ostream &operator<<(std::ostream &out, Rgb color) {
	out << "rgb(" << static_cast<int>(color.red) << ',' << static_cast<int>(color.green)
		<< ',' << static_cast<int>(color.blue) << ')';
	return out;
  }

  std::ostream &operator<<(std::ostream &out, Rgba color) {
	out << "rgba(" << static_cast<int>(color.red) << ',' << static_cast<int>(color.green)
		<< ',' << static_cast<int>(color.blue) << ',' << color.opacity << ')';
	return out;
  }

  void Object::Render(const RenderContext &context) const {
	//Шаблонный метод - паттерн
	context.RenderIndent();
	// Делегируем вывод тега своим подклассам
	RenderObject(context);
	context.out << std::endl;
  }

  // ---------- Circle ------------------

  Circle &Circle::SetCenter(Point center) {
	center_ = center;
	return *this;
  }

  Circle &Circle::SetRadius(double radius) {
	radius_ = radius;
	return *this;
  }

  void Circle::RenderObject(const RenderContext &context) const {
	auto &out = context.out;
	out << "<circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
	out << "r=\""sv << radius_ << "\""sv;
	RenderAttrs(out);
	out << "/>"sv;
  }

  Polyline &Polyline::AddPoint(Point point) {
	points_.push_back(point);
	return *this;
  }

  void Polyline::RenderObject(const RenderContext &context) const {
	auto &out = context.out;
	out << "<polyline points=\"";
	for (auto it = points_.begin(); it != points_.end(); ++it) {
	  if (it != points_.begin()) {
		out << ' ';
	  }
	  out << it->x << "," << it->y;
	}
	out << "\""sv;
	RenderAttrs(out);
	out << "/>"sv;
  }

  // ---------- Text ------------------

  Text &Text::SetPosition(Point pos) {
	position_ = pos;
	return *this;
  }

  Text &Text::SetOffset(Point offset) {
	offset_ = offset;
	return *this;
  }

  Text &Text::SetFontSize(uint32_t size) {
	font_size_ = size;
	return *this;
  }

  Text &Text::SetFontFamily(std::string font_family) {
	font_family_ = font_family;
	return *this;
  }

  Text &Text::SetFontWeight(std::string font_weight) {
	font_weight_ = font_weight;
	return *this;
  }

  Text &Text::SetData(std::string data) {
	data_ = data;
	return *this;
  }

  void Text::RenderObject(const RenderContext &context) const {
	auto &out = context.out;
	out << "<text x=\""sv << position_.x << "\" y=\""sv << position_.y << "\""sv;
	out << " dx=\""sv << offset_.x << "\" dy=\""sv << offset_.y << "\""sv;
	out << " font-size=\""sv << font_size_ << "\""sv;
	if (!font_family_.empty()) {
	  out << " font-family=\""sv << font_family_ << "\""sv;
	}
	if (!font_weight_.empty()) {
	  out << " font-weight=\""sv << font_weight_ << "\""sv;
	}
	RenderAttrs(out);
	out << ">" << data_ << "</text>";
  }

  // ---------- Document ------------------

  void Document::AddPtr(std::unique_ptr<Object> &&obj) {
	objects_.push_back(std::move(obj));
  }

  void Document::Render(std::ostream &out) const {
	out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"sv << std::endl;
	out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"sv << std::endl;
	for (const auto &obj : objects_) {
	  obj->Render(RenderContext(out));
	}
	out << "</svg>"sv;
  }

  std::ostream &operator<<(std::ostream &out, StrokeLineCap linecap) {
	switch (linecap) {
	  case StrokeLineCap::BUTT:
		out << "butt";
		break;
	  case StrokeLineCap::ROUND:
		out << "round";
		break;
	  case StrokeLineCap::SQUARE:
		out << "square";
		break;
	}
	return out;
  }

  std::ostream &operator<<(std::ostream &out, StrokeLineJoin linejoin) {
	switch (linejoin) {
	  case StrokeLineJoin::ARCS:
		out << "arcs";
		break;
	  case StrokeLineJoin::BEVEL:
		out << "bevel";
		break;
	  case StrokeLineJoin::MITER:
		out << "miter";
		break;
	  case StrokeLineJoin::MITER_CLIP:
		out << "miter-clip";
		break;
	  case StrokeLineJoin::ROUND:
		out << "round";
		break;
	}
	return out;
  }

  Rgb::Rgb(uint8_t r, uint8_t g, uint8_t b) : red(r), green(g), blue(b) {}

  Rgba::Rgba(uint8_t r, uint8_t g, uint8_t b, double a) : Rgb(r, g, b), opacity(a) {}

  void VisitorColor::operator()(const std::monostate) const {}

  void VisitorColor::operator()(Rgb color) const {
	out << color;
  }

  void VisitorColor::operator()(Rgba color) const {
	out << color;
  }

  void VisitorColor::operator()(const std::string &color) const {
	out << color;
  }

  ObjectContainer::~ObjectContainer() {}

  Drawable::~Drawable() {}

  Point::Point(double x, double y) : x(x), y(y) {}

  RenderContext::RenderContext(std::ostream &out) : out(out) {}

  RenderContext::RenderContext(std::ostream &out, int indent_step, int indent)
	  : out(out), indent_step(indent_step), indent(indent) {}

  RenderContext RenderContext::Indented() const {
	return {out, indent_step, indent + indent_step};
  }

  void RenderContext::RenderIndent() const {
	for (int i = 0; i < indent; ++i) {
	  out.put(' ');
	}
  }

}  // namespace svg
