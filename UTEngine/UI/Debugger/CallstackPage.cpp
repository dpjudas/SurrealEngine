
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
}

void CallstackPage::updateList()
{
	listview->clearList();

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
					name = s->Name;
				else
					name = s->Name + "." + name;
			}

			auto item = (TextListViewItem*)listview->rootItem()->add(std::make_unique<TextListViewItem>());
			item->setText(0, name);
			item->setText(1, std::to_string(func->Line));
		}
	}
}
