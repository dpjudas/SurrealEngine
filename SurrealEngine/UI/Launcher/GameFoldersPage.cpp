
#include "GameFoldersPage.h"
#include "LauncherWindow.h"
#include "LauncherSettings.h"
#include <zwidget/core/resourcedata.h>
#include <zwidget/widgets/textlabel/textlabel.h>
#include <zwidget/widgets/listview/listview.h>
#include <zwidget/widgets/pushbutton/pushbutton.h>
#include <zwidget/systemdialogs/open_folder_dialog.h>

GameFoldersPage::GameFoldersPage(LauncherWindow* launcher) : Widget(nullptr), Launcher(launcher)
{
	Label = new TextLabel(this);
	SearchList = new ListView(this);
	AddButton = new PushButton(this);
	RemoveButton = new PushButton(this);

	Label->SetText("Folders to scan for games:");
	AddButton->SetText("Add");
	RemoveButton->SetText("Remove");

	AddButton->OnClick = [this]() { OnAddButtonClicked(); };
	RemoveButton->OnClick = [this]() { OnRemoveButtonClicked(); };

	auto& settings = LauncherSettings::Get();
	for (auto& game : settings.Games.SearchList)
	{
		SearchList->AddItem(game);
	}
}

void GameFoldersPage::Save()
{
}

void GameFoldersPage::OnAddButtonClicked()
{
	auto dialog = OpenFolderDialog::Create(this);
	dialog->SetTitle("Select Game Folder");
	if (dialog->Show())
	{
		std::string path = dialog->SelectedPath();
		SearchList->AddItem(path);
		auto& settings = LauncherSettings::Get();
		settings.Games.SearchList.push_back(path);
		Launcher->GamesListChanged();
	}
}

void GameFoldersPage::OnRemoveButtonClicked()
{
	int item = SearchList->GetSelectedItem();
	if (item != -1)
	{
		SearchList->RemoveItem(item);
		auto& settings = LauncherSettings::Get();
		settings.Games.SearchList.erase(settings.Games.SearchList.begin() + item);
		Launcher->GamesListChanged();
	}
}

void GameFoldersPage::OnGeometryChanged()
{
	double y = 10.0;

	Label->SetFrameGeometry(0.0, y, GetWidth(), Label->GetPreferredHeight());
	y += Label->GetPreferredHeight();

	double listViewTop = y;

	y = GetHeight() - 30.0;

	AddButton->SetFrameGeometry(0.0, y, 80.0, AddButton->GetPreferredHeight());
	RemoveButton->SetFrameGeometry(85.0, y, 80.0, RemoveButton->GetPreferredHeight());

	double listViewBottom = y - 10.0;
	SearchList->SetFrameGeometry(0.0, listViewTop, GetWidth(), std::max(listViewBottom - listViewTop, 0.0));
}
