#pragma once

class Point
{
public:
	Point() = default;
	Point(double x, double y) : x(x), y(y) { }

	double x = 0;
	double y = 0;
};

class Size
{
public:
	Size() = default;
	Size(double width, double height) : width(width), height(height) { }

	double width = 0;
	double height = 0;
};

class Rect
{
public:
	Rect() = default;
	Rect(double x, double y, double width, double height) : x(x), y(y), width(width), height(height) { }

	Point pos() const { return { x, y }; }
	Size size() const { return { width, height }; }

	Point topLeft() const { return { x, y }; }
	Point topRight() const { return { x + width, y }; }
	Point bottomLeft() const { return { x, y + height }; }
	Point bottomRight() const { return { x + width, y + height }; }

	static Rect xywh(double x, double y, double width, double height) { return Rect(x, y, width, height); }
	static Rect ltrb(double left, double top, double right, double bottom) { return Rect(left, top, right - left, bottom - top); }

	bool contains(const Point& p) const { return (p.x >= x && p.x < x + width) && (p.y >= y && p.y < x + height); }

	double x = 0;
	double y = 0;
	double width = 0;
	double height = 0;
};

inline Point operator+(const Point& a, const Point& b) { return Point(a.x + b.x, a.y + b.y); }
inline Point operator-(const Point& a, const Point& b) { return Point(a.x - b.x, a.y - b.y); }
inline bool operator==(const Point& a, const Point& b) { return a.x == b.x && a.y == b.y; }
inline bool operator!=(const Point& a, const Point& b) { return a.x != b.x || a.y != b.y; }
inline bool operator==(const Size& a, const Size& b) { return a.width == b.width && a.height == b.height; }
inline bool operator!=(const Size& a, const Size& b) { return a.width != b.width || a.height != b.height; }
inline bool operator==(const Rect& a, const Rect& b) { return a.x == b.x && a.y == b.y && a.width == b.width && a.height == b.height; }
inline bool operator!=(const Rect& a, const Rect& b) { return a.x != b.x || a.y != b.y || a.width != b.width || a.height != b.height; }
