
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

	GamesList->ShowHeader(true);
	GamesList->SetColumn(0, "Game", 200.0);
	GamesList->SetColumn(1, "Version", 75.0);
	GamesList->SetColumn(2, "Path", 400.0);

	WelcomeLabel->SetText("Welcome to Surreal Engine!");
	SelectLabel->SetText("Please select a game to play:");

	UpdateList();

	auto& settings = LauncherSettings::Get();

	if (settings.Games.LastSelected >= 0 && settings.Games.LastSelected < (int)GamesList->GetItemCount())
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
	while (GamesList->GetItemCount() != 0)
		GamesList->RemoveItem((int)GamesList->GetItemCount() - 1);

	for (auto& info : GameFolderSelection::Games)
		GamesList->AddItem({info.gameName, info.gameVersionString, info.gameRootFolder});
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
