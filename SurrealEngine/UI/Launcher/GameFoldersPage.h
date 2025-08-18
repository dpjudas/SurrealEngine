#pragma once

#include <zwidget/core/widget.h>

class LauncherWindow;
class ListView;
class TextLabel;
class PushButton;

class GameFoldersPage : public Widget
{
public:
	GameFoldersPage(LauncherWindow* launcher);

	void Save();

private:
	void OnGeometryChanged() override;

	void OnAddButtonClicked();
	void OnRemoveButtonClicked();

	LauncherWindow* Launcher = nullptr;

	TextLabel* Label = nullptr;
	ListView* SearchList = nullptr;
	PushButton* AddButton = nullptr;
	PushButton* RemoveButton = nullptr;
};
