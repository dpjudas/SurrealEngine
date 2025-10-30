
#include "PlayGamePage.h"
#include "LauncherWindow.h"
#include "LauncherSettings.h"
#include "GameFolder.h"
#include <zwidget/widgets/textlabel/textlabel.h>
#include <zwidget/widgets/listview/listview.h>
#include <zwidget/widgets/lineedit/lineedit.h>

#include "zwidget/widgets/layout/vboxlayout.h"

PlayGamePage::PlayGamePage(LauncherWindow* launcher) : Widget(nullptr), Launcher(launcher)
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

	UpdateList();

	auto& settings = LauncherSettings::Get();

	if (settings.Games.LastSelected >= 0 && settings.Games.LastSelected < (int)GamesList->GetItemAmount())
	{
		GamesList->SetSelectedItem(settings.Games.LastSelected);
		GamesList->ScrollToItem(settings.Games.LastSelected);
	}

	GamesList->OnActivated = [this]() { OnGamesListActivated(); };

	auto mainLayout = new VBoxLayout();
	mainLayout->AddWidget(WelcomeLabel);
	mainLayout->AddWidget(SelectLabel);
#if defined(EXTRAARGS)
	mainLayout->AddWidget(ParametersLabel);
#endif
	mainLayout->AddWidget(GamesList);
#if defined(EXTRAARGS)
	mainLayout->AddWidget(ParametersEdit);
#endif

	SetLayout(mainLayout);
}

void PlayGamePage::UpdateList()
{
	GameFolderSelection::UpdateList();

	// GamesList->Clear(); // To do: add this to zwidget
	while (GamesList->GetItemAmount() != 0)
		GamesList->RemoveItem((int)GamesList->GetItemAmount() - 1);

	for (const GameLaunchInfo& info : GameFolderSelection::Games)
	{
		GamesList->AddItem(info.gameName + " (" + info.gameVersionString + ")");
	}
}

void PlayGamePage::Save()
{
	auto& settings = LauncherSettings::Get();
	if (GamesList->GetSelectedItem() != -1)
		settings.Games.LastSelected = GamesList->GetSelectedItem();
}

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
	/*
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
	*/
}
