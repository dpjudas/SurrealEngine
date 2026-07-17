
#include "Precomp.h"
#include "Utils/Exception.h"
#include "Utils/Logger.h"
#include "Utils/CommandLine.h"
#include "GameApp.h"
#include "GameFolder.h"
#include "Engine.h"
#include "UI/WidgetResourceData.h"
#include "UI/ErrorWindow/ErrorWindow.h"
#include "UI/Launcher/LauncherWindow.h"
#include "Utils/File.h"
#include <stdexcept>
#include <surrealwidgets/core/theme.h>
#include <surrealwidgets/window/window.h>
#include <iostream>

int GameApp::main(Array<std::string> args)
{
	auto backend = DisplayBackend::TryCreateBackend();
	DisplayBackend::Set(std::move(backend));
	InitWidgetResources();
	WidgetTheme::SetTheme(std::make_unique<DarkWidgetTheme>());

	try
	{
		CommandLine cmd(args);
		commandline = &cmd;

		if (ErrorWindow::CheckCrashReporter())
			return 0;

		if (commandline->HasArg("-h", "--help"))
		{
			std::cout << "SurrealEngine [--url=<mapname>] [--engineversion=X] [Path to game folder]\n";
			return 0;
		}

		// SE_AUTOLAUNCH=<index> starts that game straight away instead of showing the launcher, where the
		// index is the position in the list the launcher would have shown. For automated and unattended
		// runs: the launcher is otherwise unconditional - no command line argument skips it - so without
		// this the game cannot be started at all without a human clicking Play, and nothing about it can
		// be tested on a machine nobody is sitting at.
		//
		// UpdateList() is what LauncherWindow normally does to populate GameFolderSelection::Games, and
		// has to happen here too since we're not building one. It reads the same folder arguments and
		// auto-detection the launcher does, so index 0 is the game the launcher would have preselected.
		int selectedGameIndex;
		if (const char* autoLaunch = getenv("SE_AUTOLAUNCH"))
		{
			GameFolderSelection::UpdateList();
			selectedGameIndex = atoi(autoLaunch);
		}
		else
		{
			selectedGameIndex = LauncherWindow::ExecModal();
		}

		if (selectedGameIndex >= 0)
		{
			GameLaunchInfo info = GameFolderSelection::GetLaunchInfo(selectedGameIndex);
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
