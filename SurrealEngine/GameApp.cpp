
#include "Precomp.h"
#include "Exception.h"
#include "Logger.h"
#include "GameApp.h"
#include "CommandLine.h"
#include "GameFolder.h"
#include "Engine.h"
#include "UI/WidgetResourceData.h"
#include "UI/ErrorWindow/ErrorWindow.h"
#include "File.h"
#include <stdexcept>
#include <zwidget/core/theme.h>
#include <zwidget/window/window.h>

int GameApp::main(std::vector<std::string> args)
{
	auto backend = DisplayBackend::TryCreateBackend();
	DisplayBackend::Set(std::move(backend));
	InitWidgetResources();
	WidgetTheme::SetTheme(std::make_unique<DarkWidgetTheme>());

	try
	{
		CommandLine cmd(args);
		commandline = &cmd;

		GameLaunchInfo info = GameFolderSelection::GetLaunchInfo();
		if (!info.gameRootFolder.empty())
		{
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
