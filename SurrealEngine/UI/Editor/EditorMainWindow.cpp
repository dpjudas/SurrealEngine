
#include "Precomp.h"
#include "EditorMainWindow.h"
#include "EditorWorkspace.h"

EditorMainWindow::EditorMainWindow()
{
	Workspace = new EditorWorkspace(this);
	SetCentralWidget(Workspace);
}

EditorMainWindow::~EditorMainWindow()
{
}
