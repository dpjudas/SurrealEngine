#pragma once

#include "UNavigationPoint.h"

class UInventory;

class UInventorySpot : public UNavigationPoint
{
public:
	using UNavigationPoint::UNavigationPoint;

	UInventory*& markedItem() { return Value<UInventory*>(PropOffsets_InventorySpot.markedItem); }
};
