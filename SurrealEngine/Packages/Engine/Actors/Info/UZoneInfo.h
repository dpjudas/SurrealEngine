#pragma once

#include "UInfo.h"

class USound;
class UTexture;
class USkyZoneInfo;
class Ulocationid;

class UZoneInfo : public UInfo
{
public:
	using UInfo::UInfo;

	uint8_t& AmbientBrightness() { return Value<uint8_t>(PropOffsets_ZoneInfo.AmbientBrightness); }
	uint8_t& AmbientHue() { return Value<uint8_t>(PropOffsets_ZoneInfo.AmbientHue); }
	uint8_t& AmbientSaturation() { return Value<uint8_t>(PropOffsets_ZoneInfo.AmbientSaturation); }
	int& CutoffHz() { return Value<int>(PropOffsets_ZoneInfo.CutoffHz); }
	int& DamagePerSec() { return Value<int>(PropOffsets_ZoneInfo.DamagePerSec); }
	std::string& DamageString() { return Value<std::string>(PropOffsets_ZoneInfo.DamageString); }
	NameString& DamageType() { return Value<NameString>(PropOffsets_ZoneInfo.DamageType); }
	FixedArrayView<uint8_t, 6> Delay() { return FixedArray<uint8_t, 6>(PropOffsets_ZoneInfo.Delay); }
	UClass*& EntryActor() { return Value<UClass*>(PropOffsets_ZoneInfo.EntryActor); }
	USound*& EntrySound() { return Value<USound*>(PropOffsets_ZoneInfo.EntrySound); }
	UTexture*& EnvironmentMap() { return Value<UTexture*>(PropOffsets_ZoneInfo.EnvironmentMap); }
	UClass*& ExitActor() { return Value<UClass*>(PropOffsets_ZoneInfo.ExitActor); }
	USound*& ExitSound() { return Value<USound*>(PropOffsets_ZoneInfo.ExitSound); }
	Color& FogColor() { return Value<Color>(PropOffsets_ZoneInfo.FogColor); }
	float& FogDistance() { return Value<float>(PropOffsets_ZoneInfo.FogDistance); }
	FixedArrayView<uint8_t, 6> Gain() { return FixedArray<uint8_t, 6>(PropOffsets_ZoneInfo.Gain); }
	FixedArrayView<UTexture*, 12> LensFlare() { return FixedArray<UTexture*, 12>(PropOffsets_ZoneInfo.LensFlare); }
	FixedArrayView<float, 12> LensFlareOffset() { return FixedArray<float, 12>(PropOffsets_ZoneInfo.LensFlareOffset); }
	FixedArrayView<float, 12> LensFlareScale() { return FixedArray<float, 12>(PropOffsets_ZoneInfo.LensFlareScale); }
	uint8_t& MasterGain() { return Value<uint8_t>(PropOffsets_ZoneInfo.MasterGain); }
	int& MaxCarcasses() { return Value<int>(PropOffsets_ZoneInfo.MaxCarcasses); }
	uint8_t& MaxLightCount() { return Value<uint8_t>(PropOffsets_ZoneInfo.MaxLightCount); }
	int& MaxLightingPolyCount() { return Value<int>(PropOffsets_ZoneInfo.MaxLightingPolyCount); }
	uint8_t& MinLightCount() { return Value<uint8_t>(PropOffsets_ZoneInfo.MinLightCount); }
	int& MinLightingPolyCount() { return Value<int>(PropOffsets_ZoneInfo.MinLightingPolyCount); }
	int& NumCarcasses() { return Value<int>(PropOffsets_ZoneInfo.NumCarcasses); }
	USkyZoneInfo*& SkyZone() { return Value<USkyZoneInfo*>(PropOffsets_ZoneInfo.SkyZone); }
	float& SpeedOfSound() { return Value<float>(PropOffsets_ZoneInfo.SpeedOfSound); }
	float& TexUPanSpeed() { return Value<float>(PropOffsets_ZoneInfo.TexUPanSpeed); }
	float& TexVPanSpeed() { return Value<float>(PropOffsets_ZoneInfo.TexVPanSpeed); }
	vec3& ViewFlash() { return Value<vec3>(PropOffsets_ZoneInfo.ViewFlash); }
	vec3& ViewFog() { return Value<vec3>(PropOffsets_ZoneInfo.ViewFog); }
	float& ZoneFluidFriction() { return Value<float>(PropOffsets_ZoneInfo.ZoneFluidFriction); }
	vec3& ZoneGravity() { return Value<vec3>(PropOffsets_ZoneInfo.ZoneGravity); }
	float& ZoneGroundFriction() { return Value<float>(PropOffsets_ZoneInfo.ZoneGroundFriction); }
	std::string& ZoneName() { return Value<std::string>(PropOffsets_ZoneInfo.ZoneName); }
	int& ZonePlayerCount() { return Value<int>(PropOffsets_ZoneInfo.ZonePlayerCount); }
	NameString& ZonePlayerEvent() { return Value<NameString>(PropOffsets_ZoneInfo.ZonePlayerEvent); }
	NameString& ZoneTag() { return Value<NameString>(PropOffsets_ZoneInfo.ZoneTag); }
	float& ZoneTerminalVelocity() { return Value<float>(PropOffsets_ZoneInfo.ZoneTerminalVelocity); }
	vec3& ZoneVelocity() { return Value<vec3>(PropOffsets_ZoneInfo.ZoneVelocity); }
	BitfieldBool bBounceVelocity() { return BoolValue(PropOffsets_ZoneInfo.bBounceVelocity); }
	BitfieldBool bDestructive() { return BoolValue(PropOffsets_ZoneInfo.bDestructive); }
	BitfieldBool bFogZone() { return BoolValue(PropOffsets_ZoneInfo.bFogZone); }
	BitfieldBool bGravityZone() { return BoolValue(PropOffsets_ZoneInfo.bGravityZone); }
	BitfieldBool bKillZone() { return BoolValue(PropOffsets_ZoneInfo.bKillZone); }
	BitfieldBool bMoveProjectiles() { return BoolValue(PropOffsets_ZoneInfo.bMoveProjectiles); }
	BitfieldBool bNeutralZone() { return BoolValue(PropOffsets_ZoneInfo.bNeutralZone); }
	BitfieldBool bNoInventory() { return BoolValue(PropOffsets_ZoneInfo.bNoInventory); }
	BitfieldBool bPainZone() { return BoolValue(PropOffsets_ZoneInfo.bPainZone); }
	BitfieldBool bRaytraceReverb() { return BoolValue(PropOffsets_ZoneInfo.bRaytraceReverb); }
	BitfieldBool bReverbZone() { return BoolValue(PropOffsets_ZoneInfo.bReverbZone); }
	BitfieldBool bWaterZone() { return BoolValue(PropOffsets_ZoneInfo.bWaterZone); }
	Ulocationid*& locationid() { return Value<Ulocationid*>(PropOffsets_ZoneInfo.locationid); }
	// 227 additions
	NameString& SkyZoneInfoTag() { return Value<NameString>(PropOffsets_ZoneInfo.SkyZoneInfoTag); }
	NameString& SkyZoneInfoLevelID() { return Value<NameString>(PropOffsets_ZoneInfo.SkyZoneInfoLevelID); }
	float& MinWalkableZ() { return Value<float>(PropOffsets_ZoneInfo.MinWalkableZ); }
	BitfieldBool bDistanceFog() { return BoolValue(PropOffsets_ZoneInfo.bDistanceFog); }
	BitfieldBool bDistanceFogClips() { return BoolValue(PropOffsets_ZoneInfo.bDistanceFogClips); }
	BitfieldBool bRepZoneProperties() { return BoolValue(PropOffsets_ZoneInfo.bRepZoneProperties); }
	BitfieldBool bZoneBasedFog() { return BoolValue(PropOffsets_ZoneInfo.bZoneBasedFog); }
	float& DirtyShadowLevel() { return Value<float>(PropOffsets_ZoneInfo.DirtyShadowLevel); }
	EAmbients EFXAmbients() { return static_cast<EAmbients>(Value<uint8_t>(PropOffsets_ZoneInfo.EFXAmbients)); }
	vec3& EnvironmentColor() { return Value<vec3>(PropOffsets_ZoneInfo.EnvironmentColor); }
	float& EnvironmentUScale() { return Value<float>(PropOffsets_ZoneInfo.EnvironmentUScale); }
	float& EnvironmentVScale() { return Value<float>(PropOffsets_ZoneInfo.EnvironmentVScale); }
	float& FadeTime() { return Value<float>(PropOffsets_ZoneInfo.FadeTime); }
	float& FogDistanceStart() { return Value<float>(PropOffsets_ZoneInfo.FogDistanceStart); }
	FixedArrayView<uint8_t, 4> LightMapDetailLevels() { return FixedArray<uint8_t, 4>(PropOffsets_ZoneInfo.LightMapDetailLevels); }
	float& ZoneTimeDilation() { return Value<float>(PropOffsets_ZoneInfo.ZoneTimeDilation); }
	// VisibilityNotify& VisNotify() { return Value<VisibilityNotify>(PropOffsets_ZoneInfo.VisNotify); }
};
