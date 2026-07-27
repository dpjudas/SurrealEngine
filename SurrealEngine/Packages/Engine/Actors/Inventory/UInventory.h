#pragma once

#include "Packages/Engine/Actors/UActor.h"

class USound;
class UTexture;
class UMesh;
class UInventorySpot;

class UInventory : public UActor
{
public:
	using UActor::UActor;

	int& AbsorptionPriority() { return Value<int>(PropOffsets_Inventory.AbsorptionPriority); }
	USound*& ActivateSound() { return Value<USound*>(PropOffsets_Inventory.ActivateSound); }
	int& ArmorAbsorption() { return Value<int>(PropOffsets_Inventory.ArmorAbsorption); }
	uint8_t& AutoSwitchPriority() { return Value<uint8_t>(PropOffsets_Inventory.AutoSwitchPriority); }
	float& BobDamping() { return Value<float>(PropOffsets_Inventory.BobDamping); }
	int& Charge() { return Value<int>(PropOffsets_Inventory.Charge); }
	USound*& DeActivateSound() { return Value<USound*>(PropOffsets_Inventory.DeActivateSound); }
	uint8_t& FlashCount() { return Value<uint8_t>(PropOffsets_Inventory.FlashCount); }
	UTexture*& Icon() { return Value<UTexture*>(PropOffsets_Inventory.Icon); }
	uint8_t& InventoryGroup() { return Value<uint8_t>(PropOffsets_Inventory.InventoryGroup); }
	std::string& ItemArticle() { return Value<std::string>(PropOffsets_Inventory.ItemArticle); }
	UClass*& ItemMessageClass() { return Value<UClass*>(PropOffsets_Inventory.ItemMessageClass); }
	std::string& ItemName() { return Value<std::string>(PropOffsets_Inventory.ItemName); }
	std::string& M_Activated() { return Value<std::string>(PropOffsets_Inventory.M_Activated); }
	std::string& M_Deactivated() { return Value<std::string>(PropOffsets_Inventory.M_Deactivated); }
	std::string& M_Selected() { return Value<std::string>(PropOffsets_Inventory.M_Selected); }
	float& MaxDesireability() { return Value<float>(PropOffsets_Inventory.MaxDesireability); }
	UMesh*& MuzzleFlashMesh() { return Value<UMesh*>(PropOffsets_Inventory.MuzzleFlashMesh); }
	float& MuzzleFlashScale() { return Value<float>(PropOffsets_Inventory.MuzzleFlashScale); }
	uint8_t& MuzzleFlashStyle() { return Value<uint8_t>(PropOffsets_Inventory.MuzzleFlashStyle); }
	UTexture*& MuzzleFlashTexture() { return Value<UTexture*>(PropOffsets_Inventory.MuzzleFlashTexture); }
	UInventory*& NextArmor() { return Value<UInventory*>(PropOffsets_Inventory.NextArmor); }
	uint8_t& OldFlashCount() { return Value<uint8_t>(PropOffsets_Inventory.OldFlashCount); }
	std::string& PickupMessage() { return Value<std::string>(PropOffsets_Inventory.PickupMessage); }
	UClass*& PickupMessageClass() { return Value<UClass*>(PropOffsets_Inventory.PickupMessageClass); }
	USound*& PickupSound() { return Value<USound*>(PropOffsets_Inventory.PickupSound); }
	UMesh*& PickupViewMesh() { return Value<UMesh*>(PropOffsets_Inventory.PickupViewMesh); }
	float& PickupViewScale() { return Value<float>(PropOffsets_Inventory.PickupViewScale); }
	NameString& PlayerLastTouched() { return Value<NameString>(PropOffsets_Inventory.PlayerLastTouched); }
	UMesh*& PlayerViewMesh() { return Value<UMesh*>(PropOffsets_Inventory.PlayerViewMesh); }
	vec3& PlayerViewOffset() { return Value<vec3>(PropOffsets_Inventory.PlayerViewOffset); }
	float& PlayerViewScale() { return Value<float>(PropOffsets_Inventory.PlayerViewScale); }
	NameString& ProtectionType1() { return Value<NameString>(PropOffsets_Inventory.ProtectionType1); }
	NameString& ProtectionType2() { return Value<NameString>(PropOffsets_Inventory.ProtectionType2); }
	USound*& RespawnSound() { return Value<USound*>(PropOffsets_Inventory.RespawnSound); }
	float& RespawnTime() { return Value<float>(PropOffsets_Inventory.RespawnTime); }
	UTexture*& StatusIcon() { return Value<UTexture*>(PropOffsets_Inventory.StatusIcon); }
	UMesh*& ThirdPersonMesh() { return Value<UMesh*>(PropOffsets_Inventory.ThirdPersonMesh); }
	float& ThirdPersonScale() { return Value<float>(PropOffsets_Inventory.ThirdPersonScale); }
	BitfieldBool bActivatable() { return BoolValue(PropOffsets_Inventory.bActivatable); }
	BitfieldBool bActive() { return BoolValue(PropOffsets_Inventory.bActive); }
	BitfieldBool bAmbientGlow() { return BoolValue(PropOffsets_Inventory.bAmbientGlow); }
	BitfieldBool bDisplayableInv() { return BoolValue(PropOffsets_Inventory.bDisplayableInv); }
	BitfieldBool bFirstFrame() { return BoolValue(PropOffsets_Inventory.bFirstFrame); }
	BitfieldBool bHeldItem() { return BoolValue(PropOffsets_Inventory.bHeldItem); }
	BitfieldBool bInstantRespawn() { return BoolValue(PropOffsets_Inventory.bInstantRespawn); }
	BitfieldBool bIsAnArmor() { return BoolValue(PropOffsets_Inventory.bIsAnArmor); }
	BitfieldBool bMuzzleFlashParticles() { return BoolValue(PropOffsets_Inventory.bMuzzleFlashParticles); }
	BitfieldBool bRotatingPickup() { return BoolValue(PropOffsets_Inventory.bRotatingPickup); }
	BitfieldBool bSleepTouch() { return BoolValue(PropOffsets_Inventory.bSleepTouch); }
	BitfieldBool bSteadyFlash3rd() { return BoolValue(PropOffsets_Inventory.bSteadyFlash3rd); }
	BitfieldBool bSteadyToggle() { return BoolValue(PropOffsets_Inventory.bSteadyToggle); }
	BitfieldBool bToggleSteadyFlash() { return BoolValue(PropOffsets_Inventory.bToggleSteadyFlash); }
	BitfieldBool bTossedOut() { return BoolValue(PropOffsets_Inventory.bTossedOut); }
	UInventorySpot*& myMarker() { return Value<UInventorySpot*>(PropOffsets_Inventory.myMarker); }
};
