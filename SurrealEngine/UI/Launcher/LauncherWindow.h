#pragma once

#include <zwidget/core/widget.h>

class TabWidget;
class LauncherBanner;
class LauncherButtonbar;
class PlayGamePage;
class SettingsPage;
class GameFoldersPage;
struct GameLaunchInfo;

class LauncherWindow : public Widget
{
public:
	static int ExecModal(const Array<GameLaunchInfo>& games);

	LauncherWindow(const Array<GameLaunchInfo>& games);

	void Start();
	void Exit();

private:
	void OnClose() override;
	void OnGeometryChanged() override;

	LauncherBanner* Banner = nullptr;
	TabWidget* Pages = nullptr;
	LauncherButtonbar* Buttonbar = nullptr;

	PlayGamePage* PlayGame = nullptr;
	SettingsPage* Settings = nullptr;
	GameFoldersPage* GameFolders = nullptr;

	int ExecResult = -1;
};
