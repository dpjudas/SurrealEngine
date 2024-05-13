
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

int GameApp::main(std::vector<std::string> args)
{
	if (!Exception::Init())
		throw std::runtime_error("Failed to initialize exception handler");

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

	Exception::Exit();
	return 0;
}
