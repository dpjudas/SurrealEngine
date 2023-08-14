
#include "Precomp.h"
#include "EditorApp.h"
#include "CommandLine.h"
#include "GameFolder.h"
#include "Engine.h"
#include "UI/Editor/EditorMainWindow.h"

int EditorApp::main(std::vector<std::string> args)
{
	CommandLine cmd(args);
	commandline = &cmd;
	GameLaunchInfo launchinfo = GameFolderSelection::GetLaunchInfo();

	Engine engine(launchinfo);

	auto editorWindow = std::make_unique<EditorMainWindow>();
	editorWindow->SetFrameGeometry(Rect::xywh(0.0, 0.0, 1920.0, 1080.0));
	editorWindow->Show();
	while (!editorWindow->ExitFlag)
	{
		editorWindow->Repaint();
		editorWindow->TickWindow();
#ifdef WIN32
		Sleep(10); // To do: improve DisplayWindow to support waiting for messages
#endif
	}

	return 0;
}
