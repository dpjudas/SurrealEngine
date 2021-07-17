#pragma once

#include "Rect.h"

// Element position and size after being laid out by layout functions
class ElementGeometry
{
public:
	// Margin box relative to parent element
	Rect marginBox() const;

	// Border box relative to parent element
	Rect borderBox() const;

	// Padding box relative to parent element
	Rect paddingBox() const;

	// Content box relative to parent element
	Rect contentBox() const;

	// Content offset relative to parent element
	Point contentPos() const;

	// Content box size
	Size contentSize() const;

	// Scrollbar position
	Point scrollPos() const { return { scrollX, scrollY }; }

	// Left margin width
	double marginLeft = 0.0;

	// Top margin width
	double marginTop = 0.0;

	// Right margin width
	double marginRight = 0.0;

	// Bottom margin width
	double marginBottom = 0.0;

	// Left border width
	double borderLeft = 0.0;

	// Top border width
	double borderTop = 0.0;

	// Right border width
	double borderRight = 0.0;

	// Bottom border width
	double borderBottom = 0.0;

	// Left padding width
	double paddingLeft = 0.0;

	// Top padding width
	double paddingTop = 0.0;

	// Right padding width
	double paddingRight = 0.0;

	// Bottom padding width
	double paddingBottom = 0.0;

	// Content area horizontal offset relative to parent element
	double contentX = 0.0;

	// Content area vertical offset relative to parent element
	double contentY = 0.0;

	// Content area width
	double contentWidth = 0.0;

	// Content area height
	double contentHeight = 0.0;

	// Scrollbar x position
	double scrollX = 0.0;

	// Scrollbar y position
	double scrollY = 0.0;

	// Inner scroll area width
	double scrollWidth = 0.0;

	// Inner scroll area height
	double scrollHeight = 0.0;
};
