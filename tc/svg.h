#pragma once

#include <cstdint>
#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <variant>
#include <vector>

namespace svg {

  struct Rgb {
  public:
	Rgb(uint8_t r = 0, uint8_t g = 0, uint8_t b = 0);
	uint8_t red = 0;
	uint8_t green = 0;
	uint8_t blue = 0;
  };

  std::ostream& operator<<(std::ostream& out, Rgb color);

  struct Rgba : public Rgb {
  public:
	Rgba(uint8_t r = 0, uint8_t g = 0, uint8_t b = 0, double a = 1.0);
	double opacity = 1.0;
  };

  std::ostream& operator<<(std::ostream& out, Rgba color);

  using Color = std::variant<std::monostate, Rgb, Rgba, std::string>;

  inline const Color NoneColor {"none"};

  enum class StrokeLineCap {
	BUTT,
	ROUND,
	SQUARE,
  };

  std::ostream& operator<<(std::ostream& out, StrokeLineCap linecap);

  enum class StrokeLineJoin {
	ARCS,
	BEVEL,
	MITER,
	MITER_CLIP,
	ROUND,
  };

  std::ostream& operator<<(std::ostream& out, StrokeLineJoin linejoin);

  struct VisitorColor {
	std::ostream& out;
	void operator()(const std::monostate) const;
	void operator()(Rgb color) const;
	void operator()(Rgba color) const;
	void operator()(const std::string& color) const;
  };

  template <typename Owner>
  class PathProps {
  public:
	Owner& SetFillColor(Color color);
	Owner& SetStrokeColor(Color color);
	Owner& SetStrokeWidth(double width);
	Owner& SetStrokeLineCap(StrokeLineCap line_cap);
	Owner& SetStrokeLineJoin(StrokeLineJoin line_join);

  protected:
	~PathProps() = default;
	void RenderAttrs(std::ostream& out) const;

  private:
	Owner& AsOwner();

  private:
	std::optional<Color> fill_color_;
	std::optional<Color> stroke_color_;
	std::optional<double> stroke_width_;
	std::optional<StrokeLineCap> stroke_linecap_;
	std::optional<StrokeLineJoin> stroke_linejoin_;
  };

  class Object;

  class ObjectContainer {
  public:
	virtual ~ObjectContainer();
	template <typename T>
	void Add(T obj) {
	  AddPtr(std::make_unique<T>(std::move(obj)));
	}

	virtual void AddPtr(std::unique_ptr<Object>&& obj) = 0;
  };

  class Drawable {
  public:
	virtual ~Drawable();
	virtual void Draw(ObjectContainer& container) const = 0;
  };

  struct Point {
	Point() = default;
	Point(double x, double y);
	double x = 0;
	double y = 0;
  };

  struct RenderContext {
	RenderContext(std::ostream& out);
	RenderContext(std::ostream& out, int indent_step, int indent = 0);
	RenderContext Indented() const;
	void RenderIndent() const;

	std::ostream& out;
	int indent_step = 0;
	int indent = 0;
  };

  class Object {
  public:
	void Render(const RenderContext& context) const;

	virtual ~Object() = default;

  private:
	virtual void RenderObject(const RenderContext& context) const = 0;
  };

  class Circle final : public Object, public PathProps<Circle> {
  public:
	Circle& SetCenter(Point center);
	Circle& SetRadius(double radius);

  private:
	void RenderObject(const RenderContext& context) const override;

	Point center_;
	double radius_ = 1.0;
  };

  class Polyline final : public Object, public PathProps<Polyline> {
  public:
	Polyline& AddPoint(Point point);

  private:
	void RenderObject(const RenderContext& context) const override;

  private:
	std::vector<Point> points_;
  };

  class Text final : public Object, public PathProps<Text> {
  public:
	Text& SetPosition(Point pos);
	Text& SetOffset(Point offset);
	Text& SetFontSize(uint32_t size);
	Text& SetFontFamily(std::string font_family);
	Text& SetFontWeight(std::string font_weight);
	Text& SetData(std::string data);

  private:
	void RenderObject(const RenderContext& context) const override;

  private:
	Point position_;
	Point offset_;
	uint32_t font_size_ = 1;
	std::string font_family_;
	std::string font_weight_;
	std::string data_;
  };

  class Document : public ObjectContainer {
  public:
	void AddPtr(std::unique_ptr<Object>&& obj) override;
	void Render(std::ostream& out) const;

  private:
	std::vector<std::unique_ptr<Object>> objects_;
  };

}  // namespace svg

// --------------------REALIZATION-------------------------

template <typename Owner>
Owner& svg::PathProps<Owner>::SetFillColor(Color color) {
  fill_color_ = std::move(color);
  return AsOwner();
}

template <typename Owner>
Owner& svg::PathProps<Owner>::SetStrokeColor(Color color) {
  stroke_color_ = std::move(color);
  return AsOwner();
}

template <typename Owner>
Owner& svg::PathProps<Owner>::SetStrokeWidth(double width) {
  stroke_width_ = width;
  return AsOwner();
}

template <typename Owner>
Owner& svg::PathProps<Owner>::SetStrokeLineCap(StrokeLineCap line_cap) {
  stroke_linecap_ = line_cap;
  return AsOwner();
}

template <typename Owner>
Owner& svg::PathProps<Owner>::SetStrokeLineJoin(StrokeLineJoin line_join) {
  stroke_linejoin_ = line_join;
  return AsOwner();
}

template <typename Owner>
void svg::PathProps<Owner>::RenderAttrs(std::ostream& out) const {
  using namespace std::literals;

  if (fill_color_) {
	out << " fill=\""sv;
	std::visit(VisitorColor {out}, *fill_color_);
	out << "\""sv;
  }
  if (stroke_color_) {
	out << " stroke=\""sv;
	std::visit(VisitorColor {out}, *stroke_color_);
	out << "\""sv;
  }
  if (stroke_width_) {
	out << " stroke-width=\""sv << *stroke_width_ << "\""sv;
  }
  if (stroke_linecap_) {
	out << " stroke-linecap=\""sv << *stroke_linecap_ << "\""sv;
  }
  if (stroke_linejoin_) {
	out << " stroke-linejoin=\""sv << *stroke_linejoin_ << "\""sv;
  }
}

template <typename Owner>
Owner& svg::PathProps<Owner>::AsOwner() {
  return static_cast<Owner&>(*this);
}
