
#include "Precomp.h"
#include "LogPage.h"
#include "UI/Controls/ListView/ListView.h"
#include "Engine.h"

LogPage::LogPage(View* parent) : VBoxView(parent)
{
	listview = new ListView(this);
	listview->setExpanding();

	listview->addColumn("Time", 100, false);
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
				item->setText(0, toFixed(it->Time));
				item->setText(1, it->Source);
				item->setText(2, it->Text);
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
			item->setText(0, toFixed(it->Time));
			item->setText(1, it->Source);
			item->setText(2, it->Text);
			++next;
		}
	}
}

std::string LogPage::toFixed(float time)
{
	std::string fixedTime = std::to_string((int64_t)(time * 1000.0));
	if (fixedTime.size() < 4)
		fixedTime.resize(4, '0');
	return fixedTime.substr(0, fixedTime.size() - 3) + "." + fixedTime.substr(fixedTime.size() - 3);
}
