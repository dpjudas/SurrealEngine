
#include "Precomp.h"
#include "EditorWorkspace.h"
#include "EditorViewport.h"

EditorWorkspace::EditorWorkspace(Widget* parent) : Widget(parent)
{
	for (auto& viewport : Viewports)
		viewport = new EditorViewport(this);
}

EditorWorkspace::~EditorWorkspace()
{
}

void EditorWorkspace::OnGeometryChanged()
{
	Size s = GetSize() * 0.5;
	double borderSize = 4.0;

	Viewports[0]->SetFrameGeometry(0.0, 0.0, s.width - borderSize, s.height - borderSize);
	Viewports[1]->SetFrameGeometry(s.width + borderSize, 0.0, s.width - borderSize, s.height - borderSize);
	Viewports[2]->SetFrameGeometry(0.0, s.height + borderSize, s.width - borderSize, s.height - borderSize);
	Viewports[3]->SetFrameGeometry(s.width + borderSize, s.height + borderSize, s.width - borderSize, s.height - borderSize);
}
