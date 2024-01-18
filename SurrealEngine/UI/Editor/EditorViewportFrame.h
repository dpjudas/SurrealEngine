
#pragma once

#include <zwidget/core/widget.h>
#include "Math/rotator.h"

class EditorViewport;
class EditorViewportHeader;

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
	EditorViewportHeader* Header = nullptr;
	EditorViewport* Viewport = nullptr;
};
