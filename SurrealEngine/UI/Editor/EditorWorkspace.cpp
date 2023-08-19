
#include "Precomp.h"
#include "EditorWorkspace.h"
#include "EditorViewportFrame.h"

EditorWorkspace::EditorWorkspace(Widget* parent) : Widget(parent)
{
	for (auto& viewport : Viewports)
		viewport = new EditorViewportFrame(this);

	Viewports[0]->SetTopMode();
	Viewports[1]->SetFrontMode();
	Viewports[2]->Set3DMode();
	Viewports[3]->SetSideMode();
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
