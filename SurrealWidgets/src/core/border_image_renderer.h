#pragma once

#include "surrealwidgets/core/rect.h"
#include <memory>

class Image;
class Canvas;

enum class BorderImageValueType
{
	px,
	percentage,
	number
};

class BorderImageValue
{
public:
	BorderImageValue() = default;
	BorderImageValue(double value, BorderImageValueType type = BorderImageValueType::px) : type(type), v((float)value) {}

	bool is_percentage() const { return type == BorderImageValueType::percentage; }
	bool is_number() const { return type == BorderImageValueType::number; }
	bool is_length() const { return type == BorderImageValueType::px; }
	float number() const { return v; }

private:
	BorderImageValueType type = BorderImageValueType::px;
	float v = 0.0f;
};

enum class BorderImageRepeat
{
	stretch,
	repeat,
	round,
	space
};

class BorderImage
{
public:
	std::shared_ptr<Image> source;

	struct
	{
		BorderImageValue top;
		BorderImageValue bottom;
		BorderImageValue left;
		BorderImageValue right;
	} width;

	struct
	{
		BorderImageRepeat x = BorderImageRepeat::stretch;
		BorderImageRepeat y = BorderImageRepeat::stretch;
	} repeat;

	struct
	{
		BorderImageValue top;
		BorderImageValue bottom;
		BorderImageValue left;
		BorderImageValue right;
	} outset;

	struct
	{
		bool center = true; // fill
		BorderImageValue top;
		BorderImageValue bottom;
		BorderImageValue left;
		BorderImageValue right;
	} slice;
};

class BorderGeometry
{
public:
	Rect box;
	struct
	{
		double left = 0.0;
		double right = 0.0;
		double top = 0.0;
		double bottom = 0.0;
	} border;
};

class BorderImageRenderer
{
public:
	static void render(Canvas* canvas, const BorderGeometry& geometry, const BorderImage& style);

private:
	BorderImageRenderer(Canvas* canvas, const BorderGeometry& geometry, const BorderImage& style);
	void render();

	class Box
	{
	public:
		Box() = default;
		Box(const Rect& r) : left((float)r.left()), top((float)r.top()), right((float)r.right()), bottom((float)r.bottom()) {}
		Box(float l, float t, float r, float b) : left(l), top(t), right(r), bottom(b) {}

		float left = 0.0f;
		float top = 0.0f;
		float right = 0.0f;
		float bottom = 0.0f;

		float get_width() const { return right - left; }
		float get_height() const { return bottom - top; }

		Box& clip(const Box& cr)
		{
			top = std::max(top, cr.top);
			left = std::max(left, cr.left);
			right = std::min(right, cr.right);
			bottom = std::min(bottom, cr.bottom);
			top = std::min(top, bottom);
			left = std::min(left, right);
			return *this;
		}

		static Box xywh(float x, float y, float w, float h)
		{
			return Box(x, y, x + w, y + h);
		}
	};

	struct TileRepeatInfo
	{
		float start = 0.0f;
		float width = 0.0f;
		float space = 0.0f;
		int count = 1;
	};

	TileRepeatInfo repeat_info(float x, float w, int sw, BorderImageRepeat repeat_x);

	void draw_area(const std::shared_ptr<Image>& image, float x, float y, float w, float h, int sx, int sy, int sw, int sh, BorderImageRepeat repeat_x, BorderImageRepeat repeat_y);
	Box get_border_image_area() const;
	float get_left_grid(float image_area_width, float auto_width) const;
	float get_right_grid(float image_area_width, float auto_width) const;
	float get_top_grid(float image_area_height, float auto_height) const;
	float get_bottom_grid(float image_area_height, float auto_height) const;
	int get_left_slice_value(int image_width) const;
	int get_right_slice_value(int image_width) const;
	int get_top_slice_value(int image_height) const;
	int get_bottom_slice_value(int image_height) const;

	static float mix(float a, float b, float t) { return a * (1.0f - t) + b * t; }

	Canvas* canvas = nullptr;
	const BorderGeometry& geometry;
	const BorderImage& style;
};
