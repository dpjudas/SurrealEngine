
#include "SettingsPage.h"
#include "LauncherWindow.h"
#include <zwidget/core/resourcedata.h>
#include <zwidget/widgets/textlabel/textlabel.h>
#include <zwidget/widgets/listview/listview.h>
#include <zwidget/widgets/pushbutton/pushbutton.h>
#include <zwidget/systemdialogs/open_folder_dialog.h>

SettingsPage::SettingsPage(LauncherWindow* launcher) : Widget(nullptr), Launcher(launcher)
{
	Label = new TextLabel(this);
	SearchList = new ListView(this);
	AddButton = new PushButton(this);
	RemoveButton = new PushButton(this);

	SearchList->AddItem("C:\\Games\\UnrealTournament436");
	SearchList->AddItem("C:\\Games\\Steam\\steamapps\\common\\Unreal Gold");
	SearchList->AddItem("C:\\Games\\Deus Ex");

	Label->SetText("Folders to scan for games:");
	AddButton->SetText("Add");
	RemoveButton->SetText("Remove");

	AddButton->OnClick = [=]() { OnAddButtonClicked(); };
	RemoveButton->OnClick = [=]() { OnRemoveButtonClicked(); };
}

void SettingsPage::Save()
{
}

void SettingsPage::OnAddButtonClicked()
{
	auto dialog = OpenFolderDialog::Create(this);
	dialog->SetTitle("Select Game Folder");
	if (dialog->Show())
	{
		SearchList->AddItem(dialog->SelectedPath());
	}
}

void SettingsPage::OnRemoveButtonClicked()
{
	int item = SearchList->GetSelectedItem();
	if (item != -1)
	{
		SearchList->RemoveItem(item);
	}
}

void SettingsPage::OnGeometryChanged()
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
