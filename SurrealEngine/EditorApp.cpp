
#include "Precomp.h"
#include "EditorApp.h"
#include "Utils/CommandLine.h"
#include "GameFolder.h"
#include "Engine.h"
#include "LauncherSettings.h"
#include "UI/Launcher/LauncherWindow.h"
#include "UI/Editor/EditorMainWindow.h"
#include "UI/ErrorWindow/ErrorWindow.h"
#include "UI/WidgetResourceData.h"
#include <zwidget/core/theme.h>
#include <zwidget/window/window.h>

int EditorApp::main(Array<std::string> args)
{
	auto backend = DisplayBackend::TryCreateBackend();
	DisplayBackend::Set(std::move(backend));
	InitWidgetResources();
	WidgetTheme::SetTheme(std::make_unique<DarkWidgetTheme>());

	try
	{
		CommandLine cmd(args);
		commandline = &cmd;

		GameLaunchInfo info = GameFolderSelection::GetLaunchInfo(LauncherWindow::ExecModal());
		if (!info.gameRootFolder.empty())
		{
			Engine engine(info);
			engine.setEditorMode(true);

			RenderAPI api;
			switch (LauncherSettings::Get().RenderDevice.Type)
			{
			default:
			case RenderDeviceType::Vulkan: api = RenderAPI::Vulkan; break;
			case RenderDeviceType::D3D11: api = RenderAPI::D3D11; break;
			case RenderDeviceType::D3D12: api = RenderAPI::D3D12; break;
			}
			auto editorWindow = std::make_unique<EditorMainWindow>(api);
			editorWindow->SetFrameGeometry(Rect::xywh(0.0, 0.0, 1024.0, 768.0));
			editorWindow->ShowMaximized();

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
