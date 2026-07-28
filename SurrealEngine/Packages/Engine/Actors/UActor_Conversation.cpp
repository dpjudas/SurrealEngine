
#include "Precomp.h"
#include "UActor.h"
#include "Package/PackageManager.h"
#include "Packages/ConSys/UConversation.h"
#include "Packages/ConSys/UConversationList.h"
#include "Packages/ConSys/UConListItem.h"
#include "Packages/ConSys/UConItem.h"
#include "Engine.h"

void UActor::DeusExConBindEvents()
{
	auto mission = UObject::Cast<UConversationList>(engine->GetDeusExMission());
	if (!mission)
		return;

	UClass* clsConListItem = engine->packages->FindClass("ConSys.ConListItem");
	UConListItem* conListItem = nullptr;

	NameString bindName = BindName();
	if (!bindName.IsNone())
	{
		for (UConItem* item = mission->conversations(); item; item = item->Next())
		{
			auto conversation = UObject::Cast<UConversation>(item->ConObject());
			NameString conOwnerName = conversation->conOwnerName();
			if (conOwnerName == bindName)
			{
				NameString name;
				UConListItem* newItem = UObject::Cast<UConListItem>(engine->LevelPackage->NewObject(name, clsConListItem, ObjectFlags::Transient, true));
				newItem->con() = conversation;
				newItem->Next() = conListItem;
				conListItem = newItem;
			}
		}
	}

	NameString barkBindName = BarkBindName();
	if (!barkBindName.IsNone())
	{
		for (UConItem* item = mission->conversations(); item; item = item->Next())
		{
			auto conversation = UObject::Cast<UConversation>(item->ConObject());
			NameString conOwnerName = conversation->conOwnerName();
			if (conOwnerName == barkBindName)
			{
				NameString name;
				UConListItem* newItem = UObject::Cast<UConListItem>(engine->LevelPackage->NewObject(name, clsConListItem, ObjectFlags::Transient, true));
				newItem->con() = conversation;
				newItem->Next() = conListItem;
				conListItem = newItem;
			}
		}
	}

	ConListItems() = conListItem;
}
