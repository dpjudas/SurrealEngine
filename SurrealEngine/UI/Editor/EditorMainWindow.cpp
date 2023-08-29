
#include "Precomp.h"
#include "EditorMainWindow.h"
#include "EditorWorkspace.h"
#include "Engine.h"
#include "Package/PackageManager.h"
#include "UObject/UActor.h"
#include "UObject/ULevel.h"

EditorMainWindow::EditorMainWindow()
{
	Workspace = new EditorWorkspace(this);
	SetCentralWidget(Workspace);

	engine->LevelPackage = engine->packages->GetPackage("CTF-Face");
	engine->LevelInfo = UObject::Cast<ULevelInfo>(engine->LevelPackage->GetUObject("LevelInfo", "LevelInfo0"));
	engine->Level = UObject::Cast<ULevel>(engine->LevelPackage->GetUObject("Level", "MyLevel"));
	engine->CameraActor = UObject::Cast<UActor>(engine->packages->NewObject("camera", "Engine", "Camera"));

	SetWindowTitle("Surreal Editor");
}

EditorMainWindow::~EditorMainWindow()
{
}
