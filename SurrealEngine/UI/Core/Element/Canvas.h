#pragma once

class Point;
class Rect;
class Colorf;
class RenderDevice;

class Canvas
{
public:
	static std::unique_ptr<Canvas> create(RenderDevice* renderDevice);

	virtual ~Canvas() = default;

	virtual void begin(const Colorf& color) = 0;
	virtual void end() = 0;

	virtual Point getOrigin() = 0;
	virtual void setOrigin(const Point& origin) = 0;

	virtual void pushClip(const Rect& box) = 0;
	virtual void popClip() = 0;

	virtual void fillRect(const Rect& box, const Colorf& color) = 0;
	virtual void drawText(const Point& pos, const Colorf& color, const std::string& text) = 0;
	virtual Rect measureText(const std::string& text) = 0;
};
