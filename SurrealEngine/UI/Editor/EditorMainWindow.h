
#pragma once

#include <zwidget/widgets/mainwindow/mainwindow.h>
#include <zwidget/systemdialogs/open_file_dialog.h>

class EditorWorkspace;
class Menu;

class EditorMainWindow : public MainWindow
{
public:
	EditorMainWindow();
	~EditorMainWindow();

	EditorWorkspace* Workspace = nullptr;

	void OnClose() override { DisplayWindow::ExitLoop(); }

	void OnFileMenu(Menu* menu);
	void OnEditMenu(Menu* menu);
	void OnViewMenu(Menu* menu);
	void OnToolsMenu(Menu* menu);
	//void OnWindowMenu(Menu* menu);
	void OnHelpMenu(Menu* menu);

	void OnFileNew();
	void OnFileOpen();
	void OnFileSave();
	void OnFileExit();

	void OnEditUndo();
	void OnEditRedo();
	void OnEditCut();
	void OnEditCopy();
	void OnEditPaste();
	void OnEditDelete();
	void OnEditSelectAll();

	void OnViewPackages();
	void OnViewTextures();
	void OnViewMeshes();
	void OnViewBrushes();
	void OnViewActors();

	void OnToolsTheme();
	void OnToolsCustomize();
	void OnToolsOptions();

	void OnHelpHome();
	void OnHelpAbout();

private:
	void LoadMap(std::string& mapName);
	std::unique_ptr<OpenFileDialog> openFileDialog = nullptr;
};
