#include "LauncherWindow.h"
#include "LauncherBanner.h"
#include "LauncherButtonbar.h"
#include "PlayGamePage.h"
#include "SettingsPage.h"
#include <zwidget/core/resourcedata.h>
#include <zwidget/window/window.h>
#include <zwidget/widgets/tabwidget/tabwidget.h>

#if defined(EXTRAARGS)
CVAR(String, additional_parameters, "", CVAR_ARCHIVE | CVAR_NOSET | CVAR_GLOBALCONFIG);
#endif

int LauncherWindow::ExecModal(const std::vector<GameLaunchInfo>& games)
{
	Size screenSize = GetScreenSize();
	double windowWidth = 615.0;
	double windowHeight = 700.0;

	auto launcher = std::make_unique<LauncherWindow>(games);
	launcher->SetFrameGeometry((screenSize.width - windowWidth) * 0.5, (screenSize.height - windowHeight) * 0.5, windowWidth, windowHeight);
	launcher->Show();

	DisplayWindow::RunLoop();

	return launcher->ExecResult;
}

LauncherWindow::LauncherWindow(const std::vector<GameLaunchInfo>& games) : Widget(nullptr, WidgetType::Window)
{
	SetWindowTitle("Surreal Engine");

	Banner = new LauncherBanner(this);
	Pages = new TabWidget(this);
	Buttonbar = new LauncherButtonbar(this);

	PlayGame = new PlayGamePage(this, games);
	Settings = new SettingsPage(this);

	Pages->AddTab(PlayGame, "Play");
	Pages->AddTab(Settings, "Settings");

#if defined(EXTRAARGS)
	PlayGame->SetExtraArgs(static_cast<FString>(additional_parameters).GetChars());
#endif

	Pages->SetCurrentWidget(PlayGame);
	PlayGame->SetFocus();
}

void LauncherWindow::Start()
{
	Settings->Save();

#if defined(EXTRAARGS)
	std::string extraargs = PlayGame->GetExtraArgs();
	if (extraargs != static_cast<FString>(additional_parameters).GetChars())
	{
		additional_parameters = extraargs.c_str();

		// To do: restart the process like the cocoa backend is doing?
	}
#endif

	ExecResult = PlayGame->GetSelectedGame();
	DisplayWindow::ExitLoop();
}

void LauncherWindow::Exit()
{
	ExecResult = -1;
	DisplayWindow::ExitLoop();
}

void LauncherWindow::OnClose()
{
	Exit();
}

void LauncherWindow::OnGeometryChanged()
{
	double top = 0.0;
	double bottom = GetHeight();

	Banner->SetFrameGeometry(0.0, top, GetWidth(), Banner->GetPreferredHeight());
	top += Banner->GetPreferredHeight();

	bottom -= Buttonbar->GetPreferredHeight();
	Buttonbar->SetFrameGeometry(0.0, bottom, GetWidth(), Buttonbar->GetPreferredHeight());

	Pages->SetFrameGeometry(0.0, top, GetWidth(), std::max(bottom - top, 0.0));
}
