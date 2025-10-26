#pragma once

#include <zwidget/core/widget.h>

class LauncherWindow;
class PushButton;

class LauncherButtonbar : public Widget
{
public:
	LauncherButtonbar(LauncherWindow* parent);

	double GetPreferredHeight() override;
	double GetPreferredWidth() override;

private:
	void OnGeometryChanged() override;
	void OnPlayButtonClicked();
	void OnExitButtonClicked();

	LauncherWindow* GetLauncher() const;

	PushButton* PlayButton = nullptr;
	PushButton* ExitButton = nullptr;
};
