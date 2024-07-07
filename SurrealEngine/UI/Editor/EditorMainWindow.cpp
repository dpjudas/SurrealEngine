
#include "Precomp.h"
#include "EditorMainWindow.h"
#include "EditorWorkspace.h"
#include "Engine.h"
#include "Package/PackageManager.h"
#include "UObject/UActor.h"
#include "UObject/ULevel.h"
#include "Utils/File.h"
#include <zwidget/widgets/menubar/menubar.h>

EditorMainWindow::EditorMainWindow()
{
	GetMenubar()->AddItem("File", [this](Menu* menu) { OnFileMenu(menu); });
	GetMenubar()->AddItem("Edit", [this](Menu* menu) { OnEditMenu(menu); });
	GetMenubar()->AddItem("View", [this](Menu* menu) { OnViewMenu(menu); });
	GetMenubar()->AddItem("Tools", [this](Menu* menu) { OnToolsMenu(menu); });
	//GetMenubar()->AddItem("Window", [this](Menu* menu) { OnWindowMenu(menu); });
	GetMenubar()->AddItem("Help", [this](Menu* menu) { OnHelpMenu(menu); });

	Workspace = new EditorWorkspace(this);
	SetCentralWidget(Workspace);

	SetWindowTitle("Surreal Editor: " + engine->LaunchInfo.gameName + " v" + engine->LaunchInfo.gameVersionString + " - [Untitled Map]");
}

EditorMainWindow::~EditorMainWindow()
{
}

void EditorMainWindow::OnFileMenu(Menu* menu)
{
	menu->AddItem({}, "New", [this]() { OnFileNew(); });
	menu->AddItem({}, "Open", [this]() { OnFileOpen(); });
	menu->AddSeparator();
	menu->AddItem({}, "Save", [this]() { OnFileSave(); });
	menu->AddSeparator();
	menu->AddItem({}, "Exit", [this]() { OnFileExit(); });
}

void EditorMainWindow::OnEditMenu(Menu* menu)
{
	menu->AddItem({}, "Undo", [this]() { OnEditUndo(); });
	menu->AddItem({}, "Redo", [this]() { OnEditRedo(); });
	menu->AddSeparator();
	menu->AddItem({}, "Cut", [this]() { OnEditCut(); });
	menu->AddItem({}, "Copy", [this]() { OnEditCopy(); });
	menu->AddItem({}, "Paste", [this]() { OnEditPaste(); });
	menu->AddItem({}, "Delete", [this]() { OnEditDelete(); });
	menu->AddSeparator();
	menu->AddItem({}, "Select all", [this]() { OnEditSelectAll(); });
}

void EditorMainWindow::OnViewMenu(Menu* menu)
{
	menu->AddItem({}, "Packages", [this]() { OnViewPackages(); });
	menu->AddItem({}, "Textures", [this]() { OnViewTextures(); });
	menu->AddItem({}, "Meshes", [this]() { OnViewMeshes(); });
	menu->AddItem({}, "Brushes", [this]() { OnViewBrushes(); });
	menu->AddItem({}, "Actors", [this]() { OnViewActors(); });
}

void EditorMainWindow::OnToolsMenu(Menu* menu)
{
	menu->AddItem({}, "Theme", [this]() { OnToolsTheme(); });
	menu->AddItem({}, "Customize", [this]() { OnToolsCustomize(); });
	menu->AddItem({}, "Options", [this]() { OnToolsOptions(); });
}

/*
void EditorMainWindow::OnWindowMenu(Menu* menu)
{
}
*/

void EditorMainWindow::OnHelpMenu(Menu* menu)
{
	menu->AddItem({}, "View Help", [this]() { OnHelpHome(); });
	menu->AddItem({}, "About Surreal Engine", [this]() { OnHelpAbout(); });
}

void EditorMainWindow::OnFileNew()
{
}

void EditorMainWindow::OnFileOpen()
{
	auto mapExtension = engine->packages->GetMapExtension();
	openFileDialog = OpenFileDialog::Create(this);
	openFileDialog->SetTitle("Select Map");
	openFileDialog->AddFilter("Map files", "*." + mapExtension);
	openFileDialog->SetInitialDirectory(FilePath::combine(engine->LaunchInfo.gameRootFolder, "Maps"));

	if (openFileDialog->Show())
	{
		// Load the map and all that stuff
		auto mapFile = FilePath::last_component(openFileDialog->Filename());
		auto mapName = FilePath::remove_extension(mapFile);

		LoadMap(mapName);

		this->SetWindowTitle("Surreal Editor: " + engine->LaunchInfo.gameName + " v" + engine->LaunchInfo.gameVersionString + " - " + mapFile);
		// Refresh the viewports after loading the level
		Update();
	}
}

void EditorMainWindow::OnFileSave()
{
}

void EditorMainWindow::OnFileExit()
{
	Close();
}

void EditorMainWindow::OnEditUndo()
{
}

void EditorMainWindow::OnEditRedo()
{
}

void EditorMainWindow::OnEditCut()
{
}

void EditorMainWindow::OnEditCopy()
{
}

void EditorMainWindow::OnEditPaste()
{
}

void EditorMainWindow::OnEditDelete()
{
}

void EditorMainWindow::OnEditSelectAll()
{
}

void EditorMainWindow::OnViewPackages()
{
}

void EditorMainWindow::OnViewTextures()
{
}

void EditorMainWindow::OnViewMeshes()
{
}

void EditorMainWindow::OnViewBrushes()
{
}

void EditorMainWindow::OnViewActors()
{
}

void EditorMainWindow::OnToolsTheme()
{
}

void EditorMainWindow::OnToolsCustomize()
{
}

void EditorMainWindow::OnToolsOptions()
{
}

void EditorMainWindow::OnHelpHome()
{
}

void EditorMainWindow::OnHelpAbout()
{
}

void EditorMainWindow::LoadMap(std::string& mapName)
{
	engine->LevelPackage = engine->packages->GetPackage(FilePath::remove_extension(mapName));
	engine->LevelInfo = UObject::Cast<ULevelInfo>(engine->LevelPackage->GetUObject("LevelInfo", "LevelInfo0"));
	engine->Level = UObject::Cast<ULevel>(engine->LevelPackage->GetUObject("Level", "MyLevel"));
	engine->CameraActor = UObject::Cast<UActor>(engine->packages->NewObject("camera", "Engine", "Camera"));
}
