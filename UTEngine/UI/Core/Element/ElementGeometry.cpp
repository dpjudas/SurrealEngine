
#include "Precomp.h"
#include "ElementGeometry.h"

Rect ElementGeometry::marginBox() const
{
	return Rect::ltrb(
		contentX - marginLeft - borderLeft - paddingLeft,
		contentY - marginTop - borderTop - paddingTop,
		contentX + contentWidth + marginRight + borderRight + paddingRight,
		contentY + contentHeight + marginBottom + borderBottom + paddingBottom);
}

Rect ElementGeometry::borderBox() const
{
	return Rect::ltrb(
		contentX - borderLeft - paddingLeft,
		contentY - borderTop - paddingTop,
		contentX + contentWidth + borderRight + paddingRight,
		contentY + contentHeight + borderBottom + paddingBottom);
}

Rect ElementGeometry::paddingBox() const
{
	return Rect::ltrb(
		contentX - paddingLeft,
		contentY - paddingTop,
		contentX + contentWidth + paddingRight,
		contentY + contentHeight + paddingBottom);
}

Rect ElementGeometry::contentBox() const
{
	return Rect::xywh(contentX, contentY, contentWidth, contentHeight);
}

Point ElementGeometry::contentPos() const
{
	return Point(contentX, contentY);
}

Size ElementGeometry::contentSize() const
{
	return Size(contentWidth, contentHeight);
}
