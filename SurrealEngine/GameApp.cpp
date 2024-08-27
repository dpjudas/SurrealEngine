
#include "Precomp.h"
#include "Utils/Exception.h"
#include "Utils/Logger.h"
#include "GameApp.h"
#include "Utils/CommandLine.h"
#include "GameFolder.h"
#include "Engine.h"
#include "UI/WidgetResourceData.h"
#include "UI/ErrorWindow/ErrorWindow.h"
#include "Utils/File.h"
#include <stdexcept>
#include <zwidget/core/theme.h>
#include <zwidget/window/window.h>
#include <iostream>

int GameApp::main(Array<std::string> args)
{
	//auto backend = DisplayBackend::TryCreateBackend();
	auto backend = DisplayBackend::TryCreateWin32();
	if (!backend) backend = DisplayBackend::TryCreateSDL2();
	DisplayBackend::Set(std::move(backend));
	InitWidgetResources();
	WidgetTheme::SetTheme(std::make_unique<DarkWidgetTheme>());

	try
	{
		CommandLine cmd(args);
		commandline = &cmd;

		GameLaunchInfo info = GameFolderSelection::GetLaunchInfo();
		if (info.showHelp || info.gameRootFolder.empty()) {
			std::cout << "SurrealEngine [--url=<mapname>] [--engineversion=X] [Path to game folder]\n";
		} else {
			Engine engine(info);
			engine.Run();
		}
	}
	catch (const std::exception& e)
	{
		ErrorWindow::ExecModal(e.what(), Logger::Get()->GetLog());
	}

	DeinitWidgetResources();
	return 0;
}
