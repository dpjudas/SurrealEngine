
#include "LauncherButtonbar.h"
#include "LauncherWindow.h"
#include <zwidget/widgets/pushbutton/pushbutton.h>

LauncherButtonbar::LauncherButtonbar(LauncherWindow* parent) : Widget(parent)
{
	PlayButton = new PushButton(this);
	ExitButton = new PushButton(this);
	PlayButton->SetText("Play");
	ExitButton->SetText("Exit");

	PlayButton->OnClick = [this]() { OnPlayButtonClicked(); };
	ExitButton->OnClick = [this]() { OnExitButtonClicked(); };
}

double LauncherButtonbar::GetPreferredHeight()
{
	return 20.0 + std::max(PlayButton->GetPreferredHeight(), ExitButton->GetPreferredHeight());
}

double LauncherButtonbar::GetPreferredWidth()
{
	return GetWidth(); // We'd like to get the whole width for ourselves
}

void LauncherButtonbar::OnGeometryChanged()
{
	PlayButton->SetFrameGeometry(20.0, 10.0, 120.0, PlayButton->GetPreferredHeight());
	ExitButton->SetFrameGeometry(GetWidth() - 20.0 - 120.0, 10.0, 120.0, PlayButton->GetPreferredHeight());
}

void LauncherButtonbar::OnPlayButtonClicked()
{
	GetLauncher()->Start();
}

void LauncherButtonbar::OnExitButtonClicked()
{
	GetLauncher()->Exit();
}

LauncherWindow* LauncherButtonbar::GetLauncher() const
{
	return static_cast<LauncherWindow*>(Parent());
}
