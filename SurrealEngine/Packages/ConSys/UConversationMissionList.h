#pragma once

#include "UConObject.h"

class UConItem;

class UConversationMissionList : public UConObject
{
public:
	using UConObject::UConObject;

	UConItem*& missions() { return Value<UConItem*>(PropOffsets_ConversationMissionList.missions); }
};
