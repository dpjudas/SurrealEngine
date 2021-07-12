
#include "Precomp.h"
#include "DisassemblyPage.h"
#include "UI/Controls/ListView/ListView.h"

DisassemblyPage::DisassemblyPage(View* parent) : VBoxView(parent)
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

	addClass("disassemblypage");
	listview->addClass("disassemblypage-listview");
}
