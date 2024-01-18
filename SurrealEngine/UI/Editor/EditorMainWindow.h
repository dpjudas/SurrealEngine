
#pragma once

#include <zwidget/widgets/mainwindow/mainwindow.h>

class EditorWorkspace;

class EditorMainWindow : public MainWindow
{
public:
	EditorMainWindow();
	~EditorMainWindow();

	EditorWorkspace* Workspace = nullptr;

	void OnClose() override { DisplayWindow::ExitLoop(); }
};
