#pragma once

#include "UInventoryAttachment.h"

class UWeaponMuzzleFlash;
class UWeapon;

class UWeaponAttachment : public UInventoryAttachment
{
public:
	using UInventoryAttachment::UInventoryAttachment;

	BitfieldBool bCopyDisplay() { return BoolValue(PropOffsets_WeaponAttachment.bCopyDisplay); }
	float& LastUpdateTime() { return Value<float>(PropOffsets_WeaponAttachment.LastUpdateTime); }
	UWeaponMuzzleFlash*& MyMuzzleFlash() { return Value<UWeaponMuzzleFlash*>(PropOffsets_WeaponAttachment.MyMuzzleFlash); }
	UWeapon*& WeaponOwner() { return Value<UWeapon*>(PropOffsets_WeaponAttachment.WeaponOwner); }
};
