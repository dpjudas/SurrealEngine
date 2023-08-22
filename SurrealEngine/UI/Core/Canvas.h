#pragma once

class Point;
class Rect;
class Colorf;
class RenderDevice;
struct VerticalTextPosition;

class Canvas
{
public:
	static std::unique_ptr<Canvas> create(RenderDevice* renderDevice);

	virtual ~Canvas() = default;

	virtual void begin(const Colorf& color) = 0;
	virtual void end() = 0;

	virtual void begin3d() = 0;
	virtual void end3d() = 0;

	virtual Point getOrigin() = 0;
	virtual void setOrigin(const Point& origin) = 0;

	virtual void pushClip(const Rect& box) = 0;
	virtual void popClip() = 0;

	virtual void fillRect(const Rect& box, const Colorf& color) = 0;
	virtual void line(const Point& p0, const Point& p1, const Colorf& color) = 0;

	virtual void drawText(const Point& pos, const Colorf& color, const std::string& text) = 0;
	virtual Rect measureText(const std::string& text) = 0;
	virtual VerticalTextPosition verticalTextAlign() = 0;
};

struct VerticalTextPosition
{
	double top = 0.0;
	double baseline = 0.0;
	double bottom = 0.0;
};
