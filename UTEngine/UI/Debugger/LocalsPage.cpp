
#include "Precomp.h"
#include "LocalsPage.h"
#include "UI/Controls/ListView/ListView.h"
#include "UObject/UClass.h"
#include "UObject/UProperty.h"
#include "VM/Frame.h"

LocalsPage::LocalsPage(View* parent) : VBoxView(parent)
{
	listview = new ListView(this);
	listview->setExpanding();

	listview->addColumn("Name", 300);
	listview->addColumn("Value", 100, true);

	addClass("localspage");
	listview->addClass("localspage-listview");
}

void LocalsPage::setFrame(Frame* frame)
{
	listview->clearList();

	if (frame)
	{
		for (UProperty* prop : frame->Func->Properties)
		{
			void* ptr = ((uint8_t*)frame->Variables.get()) + prop->DataOffset;

			auto item = (TextListViewItem*)listview->rootItem()->add(std::make_unique<TextListViewItem>());
			item->setText(0, prop->Name);
			item->setText(1, prop->PrintValue(ptr));
		}
	}
}
