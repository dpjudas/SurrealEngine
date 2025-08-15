
#include "PlayGamePage.h"
#include "LauncherWindow.h"
#include "GameFolder.h"
#include <zwidget/widgets/textlabel/textlabel.h>
#include <zwidget/widgets/listview/listview.h>
#include <zwidget/widgets/lineedit/lineedit.h>

PlayGamePage::PlayGamePage(LauncherWindow* launcher, const Array<GameLaunchInfo>& games) : Widget(nullptr), Launcher(launcher)
{
	WelcomeLabel = new TextLabel(this);
	SelectLabel = new TextLabel(this);
#if defined(EXTRAARGS)
	ParametersLabel = new TextLabel(this);
#endif
	GamesList = new ListView(this);
#if defined(EXTRAARGS)
	ParametersEdit = new LineEdit(this);
#endif

	WelcomeLabel->SetText("Welcome to Surreal Engine!");
	SelectLabel->SetText("Please select a game to play:");

	for (const GameLaunchInfo& info : games)
	{
		GamesList->AddItem(info.gameName + " (" + info.gameVersionString + ")");
	}

	/*
	if (defaultGame != -1)
	{
		GamesList->SetSelectedItem(defaultgame);
		GamesList->ScrollToItem(defaultgame);
	}
	*/

	GamesList->OnActivated = [this]() { OnGamesListActivated(); };
}

#if defined(EXTRAARGS)
void PlayGamePage::SetExtraArgs(const std::string& args)
{
	ParametersEdit->SetText(args);
}

std::string PlayGamePage::GetExtraArgs()
{
	return ParametersEdit->GetText();
}
#endif

int PlayGamePage::GetSelectedGame()
{
	return GamesList->GetSelectedItem();
}

void PlayGamePage::OnGamesListActivated()
{
	Launcher->Start();
}

void PlayGamePage::OnSetFocus()
{
	GamesList->SetFocus();
}

void PlayGamePage::OnGeometryChanged()
{
	double y = 10.0;

	WelcomeLabel->SetFrameGeometry(0.0, y, GetWidth(), WelcomeLabel->GetPreferredHeight());
	y += WelcomeLabel->GetPreferredHeight();

	y += 10.0;

	SelectLabel->SetFrameGeometry(0.0, y, GetWidth(), SelectLabel->GetPreferredHeight());
	y += SelectLabel->GetPreferredHeight();

	double listViewTop = y;

	y = GetHeight() - 10.0;

#if defined(EXTRAARGS)
	double editHeight = 24.0;
	y -= editHeight;
	ParametersEdit->SetFrameGeometry(0.0, y, GetWidth(), editHeight);
	y -= 5.0;

	double labelHeight = ParametersLabel->GetPreferredHeight();
	y -= labelHeight;
	ParametersLabel->SetFrameGeometry(0.0, y, GetWidth(), labelHeight);
	y -= 10.0;
#endif

	double listViewBottom = y - 10.0;
	GamesList->SetFrameGeometry(0.0, listViewTop, GetWidth(), std::max(listViewBottom - listViewTop, 0.0));
}
