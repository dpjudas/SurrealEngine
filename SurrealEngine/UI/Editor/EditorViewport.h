
#pragma once

#include "UI/Core/Widget.h"

class EditorViewport : public Widget
{
public:
	EditorViewport(Widget* parent);
	~EditorViewport();

protected:
	void OnPaint(Canvas* canvas) override;
};
