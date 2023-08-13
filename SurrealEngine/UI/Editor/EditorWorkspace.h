
#pragma once

#include "UI/Core/Widget.h"

class EditorViewport;

class EditorWorkspace : public Widget
{
public:
	EditorWorkspace(Widget* parent);
	~EditorWorkspace();

	EditorViewport* GetViewport(int index) const { return Viewports[index]; }

protected:
	void OnGeometryChanged() override;

private:
	EditorViewport* Viewports[4] = {};
};
