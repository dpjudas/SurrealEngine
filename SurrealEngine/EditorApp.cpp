
#include "Precomp.h"
#include "EditorApp.h"
#include "CommandLine.h"
#include "GameFolder.h"
#include "Engine.h"
#include "UI/Editor/EditorMainWindow.h"
#include "UI/WidgetResourceData.h"
#include <zwidget/core/theme.h>
#include <zwidget/window/window.h>

int EditorApp::main(std::vector<std::string> args)
{
	auto backend = DisplayBackend::TryCreateWin32();
	if (!backend) backend = DisplayBackend::TryCreateWayland();
	if (!backend) backend = DisplayBackend::TryCreateX11();
	if (!backend) backend = DisplayBackend::TryCreateSDL2();
	DisplayBackend::Set(std::move(backend));
	InitWidgetResources();
	WidgetTheme::SetTheme(std::make_unique<LightWidgetTheme>());

	CommandLine cmd(args);
	commandline = &cmd;

	GameLaunchInfo info = GameFolderSelection::GetLaunchInfo();
	if (!info.gameRootFolder.empty())
	{
		Engine engine(info);

		auto editorWindow = std::make_unique<EditorMainWindow>();
		editorWindow->SetFrameGeometry(Rect::xywh(0.0, 0.0, 1920.0, 1080.0));
		editorWindow->Show();

		DisplayWindow::RunLoop();
	}

	DeinitWidgetResources();
	return 0;
}
