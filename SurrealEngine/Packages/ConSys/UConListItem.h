#pragma once

#include "UConObject.h"

class UConversation;

class UConListItem : public UConObject
{
public:
	using UConObject::UConObject;

	UConListItem*& Next() { return Value<UConListItem*>(PropOffsets_ConListItem.Next); }
	UConversation*& con() { return Value<UConversation*>(PropOffsets_ConListItem.con); }
};
