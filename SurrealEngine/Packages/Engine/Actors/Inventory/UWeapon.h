#pragma once

#include "UInventory.h"

class UWeapon : public UInventory
{
public:
	using UInventory::UInventory;

	float& AIRating() { return Value<float>(PropOffsets_Weapon.AIRating); }
	Rotator& AdjustedAim() { return Value<Rotator>(PropOffsets_Weapon.AdjustedAim); }
	NameString& AltDamageType() { return Value<NameString>(PropOffsets_Weapon.AltDamageType); }
	USound*& AltFireSound() { return Value<USound*>(PropOffsets_Weapon.AltFireSound); }
	UClass*& AltProjectileClass() { return Value<UClass*>(PropOffsets_Weapon.AltProjectileClass); }
	float& AltProjectileSpeed() { return Value<float>(PropOffsets_Weapon.AltProjectileSpeed); }
	float& AltRefireRate() { return Value<float>(PropOffsets_Weapon.AltRefireRate); }
	UClass*& AmmoName() { return Value<UClass*>(PropOffsets_Weapon.AmmoName); }
	//UAmmo*& AmmoType() { return Value<UAmmo*>(PropOffsets_Weapon.AmmoType); }
	USound*& CockingSound() { return Value<USound*>(PropOffsets_Weapon.CockingSound); }
	std::string& DeathMessage() { return Value<std::string>(PropOffsets_Weapon.DeathMessage); }
	vec3& FireOffset() { return Value<vec3>(PropOffsets_Weapon.FireOffset); }
	USound*& FireSound() { return Value<USound*>(PropOffsets_Weapon.FireSound); }
	float& FiringSpeed() { return Value<float>(PropOffsets_Weapon.FiringSpeed); }
	float& FlareOffset() { return Value<float>(PropOffsets_Weapon.FlareOffset); }
	float& FlashC() { return Value<float>(PropOffsets_Weapon.FlashC); }
	float& FlashLength() { return Value<float>(PropOffsets_Weapon.FlashLength); }
	float& FlashO() { return Value<float>(PropOffsets_Weapon.FlashO); }
	int& FlashS() { return Value<int>(PropOffsets_Weapon.FlashS); }
	float& FlashTime() { return Value<float>(PropOffsets_Weapon.FlashTime); }
	float& FlashY() { return Value<float>(PropOffsets_Weapon.FlashY); }
	UTexture*& MFTexture() { return Value<UTexture*>(PropOffsets_Weapon.MFTexture); }
	float& MaxTargetRange() { return Value<float>(PropOffsets_Weapon.MaxTargetRange); }
	std::string& MessageNoAmmo() { return Value<std::string>(PropOffsets_Weapon.MessageNoAmmo); }
	USound*& Misc1Sound() { return Value<USound*>(PropOffsets_Weapon.Misc1Sound); }
	USound*& Misc2Sound() { return Value<USound*>(PropOffsets_Weapon.Misc2Sound); }
	USound*& Misc3Sound() { return Value<USound*>(PropOffsets_Weapon.Misc3Sound); }
	UTexture*& MuzzleFlare() { return Value<UTexture*>(PropOffsets_Weapon.MuzzleFlare); }
	float& MuzzleScale() { return Value<float>(PropOffsets_Weapon.MuzzleScale); }
	NameString& MyDamageType() { return Value<NameString>(PropOffsets_Weapon.MyDamageType); }
	Color& NameColor() { return Value<Color>(PropOffsets_Weapon.NameColor); }
	int& PickupAmmoCount() { return Value<int>(PropOffsets_Weapon.PickupAmmoCount); }
	UClass*& ProjectileClass() { return Value<UClass*>(PropOffsets_Weapon.ProjectileClass); }
	float& ProjectileSpeed() { return Value<float>(PropOffsets_Weapon.ProjectileSpeed); }
	float& RefireRate() { return Value<float>(PropOffsets_Weapon.RefireRate); }
	uint8_t& ReloadCount() { return Value<uint8_t>(PropOffsets_Weapon.ReloadCount); }
	USound*& SelectSound() { return Value<USound*>(PropOffsets_Weapon.SelectSound); }
	float& aimerror() { return Value<float>(PropOffsets_Weapon.aimerror); }
	BitfieldBool bAltInstantHit() { return BoolValue(PropOffsets_Weapon.bAltInstantHit); }
	BitfieldBool bAltWarnTarget() { return BoolValue(PropOffsets_Weapon.bAltWarnTarget); }
	BitfieldBool bCanThrow() { return BoolValue(PropOffsets_Weapon.bCanThrow); }
	BitfieldBool bChangeWeapon() { return BoolValue(PropOffsets_Weapon.bChangeWeapon); }
	BitfieldBool bDrawMuzzleFlash() { return BoolValue(PropOffsets_Weapon.bDrawMuzzleFlash); }
	BitfieldBool bHideWeapon() { return BoolValue(PropOffsets_Weapon.bHideWeapon); }
	BitfieldBool bInstantHit() { return BoolValue(PropOffsets_Weapon.bInstantHit); }
	BitfieldBool bLockedOn() { return BoolValue(PropOffsets_Weapon.bLockedOn); }
	BitfieldBool bMeleeWeapon() { return BoolValue(PropOffsets_Weapon.bMeleeWeapon); }
	uint8_t& bMuzzleFlash() { return Value<uint8_t>(PropOffsets_Weapon.bMuzzleFlash); }
	BitfieldBool bOwnsCrosshair() { return BoolValue(PropOffsets_Weapon.bOwnsCrosshair); }
	BitfieldBool bPointing() { return BoolValue(PropOffsets_Weapon.bPointing); }
	BitfieldBool bRapidFire() { return BoolValue(PropOffsets_Weapon.bRapidFire); }
	BitfieldBool bRecommendAltSplashDamage() { return BoolValue(PropOffsets_Weapon.bRecommendAltSplashDamage); }
	BitfieldBool bRecommendSplashDamage() { return BoolValue(PropOffsets_Weapon.bRecommendSplashDamage); }
	BitfieldBool bSetFlashTime() { return BoolValue(PropOffsets_Weapon.bSetFlashTime); }
	BitfieldBool bSpecialIcon() { return BoolValue(PropOffsets_Weapon.bSpecialIcon); }
	BitfieldBool bSplashDamage() { return BoolValue(PropOffsets_Weapon.bSplashDamage); }
	BitfieldBool bWarnTarget() { return BoolValue(PropOffsets_Weapon.bWarnTarget); }
	BitfieldBool bWeaponStay() { return BoolValue(PropOffsets_Weapon.bWeaponStay); }
	BitfieldBool bWeaponUp() { return BoolValue(PropOffsets_Weapon.bWeaponUp); }
	float& shakemag() { return Value<float>(PropOffsets_Weapon.shakemag); }
	float& shaketime() { return Value<float>(PropOffsets_Weapon.shaketime); }
	float& shakevert() { return Value<float>(PropOffsets_Weapon.shakevert); }
};
