
#include "Precomp.h"
#include "Exception.h"
#include "GameApp.h"
#include "CommandLine.h"
#include "GameFolder.h"
#include "Engine.h"
#include "UI/WidgetResourceData.h"
#include "File.h"
#include <stdexcept>
#include <zwidget/core/theme.h>
#include <zwidget/window/window.h>

int GameApp::main(std::vector<std::string> args)
{
	auto backend = DisplayBackend::TryCreateWin32();
	if (!backend) backend = DisplayBackend::TryCreateWayland();
	if (!backend) backend = DisplayBackend::TryCreateX11();
	if (!backend) backend = DisplayBackend::TryCreateSDL2();
	DisplayBackend::Set(std::move(backend));
	InitWidgetResources();
	WidgetTheme::SetTheme(std::make_unique<DarkWidgetTheme>());

	CommandLine cmd(args);
	commandline = &cmd;

	GameLaunchInfo info = GameFolderSelection::GetLaunchInfo();
	if (!info.gameRootFolder.empty())
	{
		Engine engine(info);
		engine.Run();
	}

	DeinitWidgetResources();
	return 0;
}
