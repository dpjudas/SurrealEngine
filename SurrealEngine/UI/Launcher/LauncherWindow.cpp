#include "LauncherWindow.h"
#include "LauncherBanner.h"
#include "LauncherButtonbar.h"
#include "PlayGamePage.h"
#include "SettingsPage.h"
#include "GameFoldersPage.h"
#include "LauncherSettings.h"
#include <zwidget/core/resourcedata.h>
#include <zwidget/window/window.h>
#include <zwidget/widgets/tabwidget/tabwidget.h>

int LauncherWindow::ExecModal()
{
	Size screenSize = GetScreenSize();
	double windowWidth = 676.0;
	double windowHeight = 770.0;

	auto launcher = std::make_unique<LauncherWindow>();
	launcher->SetFrameGeometry((screenSize.width - windowWidth) * 0.5, (screenSize.height - windowHeight) * 0.5, windowWidth, windowHeight);
	launcher->Show();

	DisplayWindow::RunLoop();

	return launcher->ExecResult;
}

LauncherWindow::LauncherWindow() : Widget(nullptr, WidgetType::Window)
{
	SetWindowTitle("Surreal Engine");

	Banner = new LauncherBanner(this);
	Pages = new TabWidget(this);
	Buttonbar = new LauncherButtonbar(this);

	PlayGame = new PlayGamePage(this);
	Settings = new SettingsPage(this);
	GameFolders = new GameFoldersPage(this);

	Pages->AddTab(PlayGame, "Play");
	Pages->AddTab(Settings, "Settings");
	Pages->AddTab(GameFolders, "Games");

	Pages->SetCurrentWidget(PlayGame);
	PlayGame->SetFocus();
}

void LauncherWindow::Save()
{
	PlayGame->Save();
	Settings->Save();
	GameFolders->Save();
	LauncherSettings::Get().Save();
}

void LauncherWindow::GamesListChanged()
{
	PlayGame->UpdateList();
}

void LauncherWindow::Start()
{
	Save();
	ExecResult = PlayGame->GetSelectedGame();
	DisplayWindow::ExitLoop();
}

void LauncherWindow::Exit()
{
	Save();
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
