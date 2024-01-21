
#include "Precomp.h"
#include "GameApp.h"
#include "CommandLine.h"
#include "GameFolder.h"
#include "Engine.h"
#include "UObject/NativeObjExtractor.h"
#include "VM/NativeFuncExtractor.h"
#include "UI/WidgetResourceData.h"
#include "File.h"
#include <zwidget/core/theme.h>

int GameApp::main(std::vector<std::string> args)
{
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
