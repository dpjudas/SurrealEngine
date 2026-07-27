#pragma once

#include "UXParticleEmitter.h"

class UXRainRestrictionVolume;
class UXEmitter;

enum class EWeatherAreaType : uint8_t
{
	EWA_Box, // Use Location + AppearArea for rain appearance area
	EWA_Zone // Use current zone as the appearance area
};

enum class EFallingType : uint8_t
{
	EWF_Rain,
	EWF_Snow,
	EWF_Dust,
	EWF_Neither
};

class UXWeatherEmitter : public UXParticleEmitter
{
public:
	using UXParticleEmitter::UXParticleEmitter;

	RangeVector*& AppearArea() { return Value<RangeVector*>(PropOffsets_XWeatherEmitter.AppearArea); }
	EWeatherAreaType AppearAreaType() { return static_cast<EWeatherAreaType>(Value<uint8_t>(PropOffsets_XWeatherEmitter.AppearAreaType)); }
	BitfieldBool bIsEnabled() { return BoolValue(PropOffsets_XWeatherEmitter.bIsEnabled); }
	BitfieldBool bParticleColorEnabled() { return BoolValue(PropOffsets_XWeatherEmitter.bParticleColorEnabled); }
	BitfieldBool bUseAreaSpawns() { return BoolValue(PropOffsets_XWeatherEmitter.bUseAreaSpawns); }
	Coords*& CachedCoords() { return Value<Coords*>(PropOffsets_XWeatherEmitter.CachedCoords); }
	FloatRange*& FadeOutDistance() { return Value<FloatRange*>(PropOffsets_XWeatherEmitter.FadeOutDistance); }
	vec3*& LastCamPosition() { return Value<vec3*>(PropOffsets_XWeatherEmitter.LastCamPosition); }
	FloatRange*& Lifetime() { return Value<FloatRange*>(PropOffsets_XWeatherEmitter.Lifetime); }
	float& NextParticleTime() { return Value<float>(PropOffsets_XWeatherEmitter.NextParticleTime); }
	TypedScriptArray<UXRainRestrictionVolume*> NoRainBounds() { return DynamicArray<UXRainRestrictionVolume*>(PropOffsets_XWeatherEmitter.NoRainBounds); }
	int& ParticleCount() { return Value<int>(PropOffsets_XWeatherEmitter.ParticleCount); }
	RangeVector*& ParticlesColor() { return Value<RangeVector*>(PropOffsets_XWeatherEmitter.ParticlesColor); }
	uint8_t& ParticleTexCount() { return Value<uint8_t>(PropOffsets_XWeatherEmitter.ParticleTexCount); }
	ERenderStyle PartStyle() { return static_cast<ERenderStyle>(Value<uint8_t>(PropOffsets_XWeatherEmitter.PartStyle)); }
	FixedArrayView<UTexture*, 8> PartTextures() { return FixedArray<UTexture*, 8>(PropOffsets_XWeatherEmitter.PartTextures); }
	RangeVector*& Position() { return Value<RangeVector*>(PropOffsets_XWeatherEmitter.Position); }
	UMesh*& SheetModel() { return Value<UMesh*>(PropOffsets_XWeatherEmitter.SheetModel); }
	FloatRange*& Size() { return Value<FloatRange*>(PropOffsets_XWeatherEmitter.Size); }
	float& SpawnInterval() { return Value<float>(PropOffsets_XWeatherEmitter.SpawnInterval); }
	FloatRange*& Speed() { return Value<FloatRange*>(PropOffsets_XWeatherEmitter.Speed); }
	Coords*& TransfrmCoords() { return Value<Coords*>(PropOffsets_XWeatherEmitter.TransfrmCoords); }
	FixedArrayView<vec3*, 2> VecArea() { return FixedArray<vec3*, 2>(PropOffsets_XWeatherEmitter.VecArea); }
	NameString& WallHitEmitter() { return Value<NameString>(PropOffsets_XWeatherEmitter.WallHitEmitter); }
	TypedScriptArray<UXEmitter*> WallHitEmitters() { return DynamicArray<UXEmitter*>(PropOffsets_XWeatherEmitter.WallHitEmitters); }
	EHitEventType WallHitEvent() { return static_cast<EHitEventType>(Value<uint8_t>(PropOffsets_XWeatherEmitter.WallHitEvent)); }
	float& WallHitMinZ() { return Value<float>(PropOffsets_XWeatherEmitter.WallHitMinZ); }
	NameString& WaterHitEmitter() { return Value<NameString>(PropOffsets_XWeatherEmitter.WaterHitEmitter); }
	TypedScriptArray<UXEmitter*> WaterHitEmitters() { return DynamicArray<UXEmitter*>(PropOffsets_XWeatherEmitter.WaterHitEmitters); }
	EHitEventType WaterHitEvent() { return static_cast<EHitEventType>(Value<uint8_t>(PropOffsets_XWeatherEmitter.WaterHitEvent)); }
	EFallingType WeatherType() { return static_cast<EFallingType>(Value<uint8_t>(PropOffsets_XWeatherEmitter.WeatherType)); }
};
