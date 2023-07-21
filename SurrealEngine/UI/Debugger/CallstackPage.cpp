
#include "Precomp.h"
#include "CallstackPage.h"
#include "UI/Controls/ListView/ListView.h"
#include "UObject/UClass.h"
#include "UObject/UProperty.h"
#include "VM/Frame.h"

CallstackPage::CallstackPage(View* parent) : VBoxView(parent)
{
	listview = new ListView(this);
	listview->setExpanding();

	listview->addColumn("Name", 300, true);
	listview->addColumn("Line", 100);

	addClass("callstackpage");
	listview->addClass("callstackpage-listview");

	listview->activated = [=](ListViewItem* item) { onActivated(item); };
}

void CallstackPage::onActivated(ListViewItem* item)
{
	if (activated)
	{
		int index = std::atoi(item->id().c_str());

		int i = 0;
		for (auto it = Frame::Callstack.rbegin(); it != Frame::Callstack.rend(); ++it)
		{
			Frame* frame = *it;
			if (i == index)
			{
				activated(frame);
				break;
			}

			i++;
		}
	}
}

void CallstackPage::updateList()
{
	listview->clearList();

	int index = 0;
	for (auto it = Frame::Callstack.rbegin(); it != Frame::Callstack.rend(); ++it)
	{
		Frame* frame = *it;
		UStruct* func = frame->Func;
		if (func)
		{
			std::string name;
			for (UStruct* s = func; s != nullptr; s = s->StructParent)
			{
				if (name.empty())
					name = s->Name.ToString();
				else
					name = s->Name.ToString() + "." + name;
			}

			auto item = (TextListViewItem*)listview->rootItem()->add(std::make_unique<TextListViewItem>());
			item->setId(std::to_string(index));
			item->setText(0, name);
			item->setText(1, std::to_string(func->Line));
		}
		index++;
	}
}
