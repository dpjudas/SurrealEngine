#pragma once

#include "UInventoryAttachment.h"

class UWeaponMuzzleFlash : public UInventoryAttachment
{
public:
	using UInventoryAttachment::UInventoryAttachment;

	BitfieldBool bConstantMuzzle() { return BoolValue(PropOffsets_WeaponMuzzleFlash.bConstantMuzzle); }
	BitfieldBool bStrobeMuzzle() { return BoolValue(PropOffsets_WeaponMuzzleFlash.bStrobeMuzzle); }
	BitfieldBool bFlashTimer() { return BoolValue(PropOffsets_WeaponMuzzleFlash.bFlashTimer); }
	BitfieldBool bCurrentlyVisible() { return BoolValue(PropOffsets_WeaponMuzzleFlash.bCurrentlyVisible); }
};
