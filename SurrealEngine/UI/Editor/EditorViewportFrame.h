
#pragma once

#include "UI/Core/Widget.h"
#include "Math/rotator.h"

class EditorViewport;

class EditorViewportFrame : public Widget
{
public:
	EditorViewportFrame(Widget* parent);
	~EditorViewportFrame();

	void SetTopMode();
	void SetFrontMode();
	void SetSideMode();
	void Set3DMode();

protected:
	void OnPaint(Canvas* canvas) override;
	void OnGeometryChanged() override;

private:
	EditorViewport* Viewport = nullptr;
};
