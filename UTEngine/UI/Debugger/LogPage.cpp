
#include "Precomp.h"
#include "LogPage.h"
#include "UI/Controls/ListView/ListView.h"
#include "Engine.h"

LogPage::LogPage(View* parent) : VBoxView(parent)
{
	listview = new ListView(this);
	listview->setExpanding();

	listview->addColumn("Source", 350, false);
	listview->addColumn("Message", 200, true);

	addClass("Logpage");
	listview->addClass("Logpage-listview");
}

void LogPage::update()
{
	if (!listview->rootItem()->firstChild())
	{
		it = engine->Log.begin();
		if (!engine->Log.empty())
		{
			while (true)
			{
				auto item = (TextListViewItem*)listview->rootItem()->add(std::make_unique<TextListViewItem>());
				item->setText(0, it->Source);
				item->setText(1, it->Text);
				auto next = it;
				++next;
				if (next == engine->Log.end())
					break;
				it = next;
			}
		}
	}
	else
	{
		auto next = it;
		++next;
		while (next != engine->Log.end())
		{
			it = next;
			auto item = (TextListViewItem*)listview->rootItem()->add(std::make_unique<TextListViewItem>());
			item->setText(0, it->Source);
			item->setText(1, it->Text);
			++next;
		}
	}
}
