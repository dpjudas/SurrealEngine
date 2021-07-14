
#include "Precomp.h"
#include "ObjectViewerPage.h"
#include "UI/Controls/ListView/ListView.h"
#include "UObject/UClass.h"
#include "UObject/UProperty.h"

ObjectViewerPage::ObjectViewerPage(View* parent) : VBoxView(parent)
{
	listview = new ListView(this);
	listview->setExpanding();

	listview->addColumn("Name", 200);
	listview->addColumn("Value", 200, true);

	addClass("objectviewerpage");
	listview->addClass("objectviewerpage-listview");
}

void ObjectViewerPage::setObject(UObject* obj)
{
	listview->clearList();

	for (UProperty* prop : obj->PropertyData.Class->Properties)
	{
		void* ptr = obj->PropertyData.Ptr(prop);

		auto item = (TextListViewItem*)listview->rootItem()->add(std::make_unique<TextListViewItem>());
		item->setText(0, prop->Name);
		item->setText(1, prop->PrintValue(ptr));
	}
}
