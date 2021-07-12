#pragma once

class Point;
class Rect;
class Colorf;

class Canvas
{
public:
	virtual ~Canvas() = default;

	virtual Point getOrigin() = 0;
	virtual void setOrigin(const Point& origin) = 0;

	virtual void fillRect(const Rect& box, const Colorf& color) = 0;
};
