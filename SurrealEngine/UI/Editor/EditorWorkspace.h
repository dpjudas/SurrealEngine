
#pragma once

#include "UI/Core/Widget.h"

class EditorViewportFrame;

class EditorWorkspace : public Widget
{
public:
	EditorWorkspace(Widget* parent);
	~EditorWorkspace();

	EditorViewportFrame* GetViewport(int index) const { return Viewports[index]; }

protected:
	void OnGeometryChanged() override;

private:
	EditorViewportFrame* Viewports[4] = {};
};
