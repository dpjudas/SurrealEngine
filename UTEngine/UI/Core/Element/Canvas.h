#pragma once

class Point;

class Canvas
{
public:
	virtual ~Canvas() = default;

	virtual Point getOrigin() = 0;
	virtual void setOrigin(const Point& origin) = 0;
};
