
#include "Precomp.h"
#include "EditorApp.h"
#include "CommandLine.h"
#include "GameFolder.h"
#include "Engine.h"
#include "UI/Editor/EditorMainWindow.h"
#include "UI/ErrorWindow/ErrorWindow.h"
#include "UI/WidgetResourceData.h"
#include <zwidget/core/theme.h>
#include <zwidget/window/window.h>

int EditorApp::main(std::vector<std::string> args)
{
	auto backend = DisplayBackend::TryCreateBackend();
	DisplayBackend::Set(std::move(backend));
	InitWidgetResources();
	WidgetTheme::SetTheme(std::make_unique<LightWidgetTheme>());

	try
	{
		CommandLine cmd(args);
		commandline = &cmd;

		GameLaunchInfo info = GameFolderSelection::GetLaunchInfo();
		if (!info.gameRootFolder.empty())
		{
			Engine engine(info);

			auto editorWindow = std::make_unique<EditorMainWindow>();
			editorWindow->SetFrameGeometry(Rect::xywh(0.0, 0.0, 1024.0, 768.0));
			editorWindow->Show();

			DisplayWindow::RunLoop();
		}
	}
	catch (const std::exception& e)
	{
		ErrorWindow::ExecModal(e.what(), Logger::Get()->GetLog());
	}

	DeinitWidgetResources();
	return 0;
}
