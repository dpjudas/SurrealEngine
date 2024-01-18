#pragma once

#include <zwidget/core/widget.h>

class LauncherWindow;

class SettingsPage : public Widget
{
public:
	SettingsPage(LauncherWindow* launcher);

	void Save();

private:
	void OnGeometryChanged() override;

	LauncherWindow* Launcher = nullptr;
};
