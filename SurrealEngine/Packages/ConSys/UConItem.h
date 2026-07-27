#pragma once

#include "UConObject.h"

class UConItem : public UConObject
{
public:
	using UConObject::UConObject;

	UConObject*& ConObject() { return Value<UConObject*>(PropOffsets_ConItem.ConObject); }
	UConItem*& Next() { return Value<UConItem*>(PropOffsets_ConItem.Next); }
};
