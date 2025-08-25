#pragma once

#include <zwidget/core/widget.h>

// #define EXTRAARGS

class LauncherWindow;
class TextLabel;
class ListView;
class LineEdit;
struct GameLaunchInfo;

class PlayGamePage : public Widget
{
public:
	PlayGamePage(LauncherWindow* launcher);

	int GetSelectedGame();
	void UpdateList();
	void Save();

private:
	void OnGeometryChanged() override;
	void OnSetFocus() override;
	void OnGamesListActivated();

	LauncherWindow* Launcher = nullptr;

	TextLabel* WelcomeLabel = nullptr;
	TextLabel* SelectLabel = nullptr;
#if defined(EXTRAARGS)
	TextLabel* ParametersLabel = nullptr;
#endif
	ListView* GamesList = nullptr;
#if defined(EXTRAARGS)
	LineEdit* ParametersEdit = nullptr;
#endif
};
