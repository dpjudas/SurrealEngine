#pragma once

#include "UI/Core/Widget.h"

class EditorViewportHeader : public Widget
{
public:
	EditorViewportHeader(Widget* parent);

	void SetTitle(const std::string& title);

protected:
	void OnPaint(Canvas* canvas) override;

private:
	std::string Title;
};
