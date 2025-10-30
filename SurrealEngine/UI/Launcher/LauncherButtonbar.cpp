
#include "LauncherButtonbar.h"
#include "LauncherWindow.h"
#include <zwidget/widgets/pushbutton/pushbutton.h>
#include <zwidget/widgets/layout/hboxlayout.h>

LauncherButtonbar::LauncherButtonbar(LauncherWindow* parent) : Widget(parent)
{
	PlayButton = new PushButton(this);
	ExitButton = new PushButton(this);
	PlayButton->SetText("Play");
	ExitButton->SetText("Exit");

	PlayButton->OnClick = [this]() { OnPlayButtonClicked(); };
	ExitButton->OnClick = [this]() { OnExitButtonClicked(); };

	SetNoncontentSizes(20, 5, 20, 10);

	auto mainLayout = new HBoxLayout();

	mainLayout->AddWidget(PlayButton);
	mainLayout->AddStretch();
	mainLayout->AddWidget(ExitButton);

	SetLayout(mainLayout);
}

double LauncherButtonbar::GetPreferredHeight()
{
	return GetNoncontentTop() + std::max(PlayButton->GetPreferredHeight(), ExitButton->GetPreferredHeight()) + GetNoncontentBottom();
}

double LauncherButtonbar::GetPreferredWidth()
{
	return GetWidth(); // We'd like to get the whole width for ourselves
}

void LauncherButtonbar::OnGeometryChanged()
{
	/*
	PlayButton->SetFrameGeometry(20.0, 10.0, 120.0, PlayButton->GetPreferredHeight());
	ExitButton->SetFrameGeometry(GetWidth() - 20.0 - 120.0, 10.0, 120.0, PlayButton->GetPreferredHeight());
	*/
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
