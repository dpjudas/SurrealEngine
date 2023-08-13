
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
}
