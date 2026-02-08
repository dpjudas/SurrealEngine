#pragma once

#include <zwidget/core/widget.h>

// class AudioSettingsPage;
class VideoSettingsPage;
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
	static int ExecModal();

	LauncherWindow();

	void Start();
	void Exit();
	void GamesListChanged();

private:
	void Save();
	void OnClose() override;

	LauncherBanner* Banner = nullptr;
	TabWidget* Pages = nullptr;
	LauncherButtonbar* Buttonbar = nullptr;

	PlayGamePage* PlayGame = nullptr;
	VideoSettingsPage* GraphicsSettings = nullptr;
	// AudioSettingsPage* AudioSettings = nullptr;
	GameFoldersPage* GameFolders = nullptr;

	int ExecResult = -1;
};
