
#include "Precomp.h"
#include "ObjectViewerPage.h"
#include "UI/Controls/ListView/ListView.h"

ObjectViewerPage::ObjectViewerPage(View* parent) : VBoxView(parent)
{
	listview = new ListView(this);
	listview->setExpanding();

	listview->addColumn("Name", 500);
	listview->addColumn("Value", 200);
	for (int i = 0; i < 5; i++)
	{
		auto item = (TextListViewItem*)listview->rootItem()->add(std::make_unique<TextListViewItem>());
		item->setText(0, "Foobar");
		item->setText(1, "Moo");
	}

	addClass("objectviewerpage");
	listview->addClass("objectviewerpage-listview");
}
