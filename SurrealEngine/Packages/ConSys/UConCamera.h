#pragma once

#include "UConObject.h"

class UActor;
class UConLight;

class UConCamera : public UConObject
{
public:
	using UConObject::UConObject;

	Rotator& Rotation() { return Value<Rotator>(PropOffsets_ConCamera.Rotation); }
	BitfieldBool bCameraLocationSaved() { return BoolValue(PropOffsets_ConCamera.bCameraLocationSaved); }
	BitfieldBool bDebug() { return BoolValue(PropOffsets_ConCamera.bDebug); }
	BitfieldBool bInteractiveCamera() { return BoolValue(PropOffsets_ConCamera.bInteractiveCamera); }
	BitfieldBool bUsingFallback() { return BoolValue(PropOffsets_ConCamera.bUsingFallback); }
	UActor*& cameraActor() { return Value<UActor*>(PropOffsets_ConCamera.cameraActor); }
	uint8_t& cameraFallbackPositions() { return Value<uint8_t>(PropOffsets_ConCamera.cameraFallbackPositions); }
	uint8_t& cameraHeightPositions() { return Value<uint8_t>(PropOffsets_ConCamera.cameraHeightPositions); }
	uint8_t& cameraMode() { return Value<uint8_t>(PropOffsets_ConCamera.cameraMode); }
	vec3& cameraOffset() { return Value<vec3>(PropOffsets_ConCamera.cameraOffset); }
	uint8_t& cameraPosition() { return Value<uint8_t>(PropOffsets_ConCamera.cameraPosition); }
	uint8_t& cameraType() { return Value<uint8_t>(PropOffsets_ConCamera.cameraType); }
	float& centerModifier() { return Value<float>(PropOffsets_ConCamera.centerModifier); }
	UConLight*& conLightSpeaker() { return Value<UConLight*>(PropOffsets_ConCamera.conLightSpeaker); }
	UConLight*& conLightSpeakingTo() { return Value<UConLight*>(PropOffsets_ConCamera.conLightSpeakingTo); }
	float& cosAngle() { return Value<float>(PropOffsets_ConCamera.cosAngle); }
	int& currentFallback() { return Value<int>(PropOffsets_ConCamera.currentFallback); }
	float& distanceMultiplier() { return Value<float>(PropOffsets_ConCamera.distanceMultiplier); }
	UActor*& firstActor() { return Value<UActor*>(PropOffsets_ConCamera.firstActor); }
	int& firstActorRotation() { return Value<int>(PropOffsets_ConCamera.firstActorRotation); }
	float& heightFallbackTrigger() { return Value<float>(PropOffsets_ConCamera.heightFallbackTrigger); }
	float& heightModifier() { return Value<float>(PropOffsets_ConCamera.heightModifier); }
	BitfieldBool ignoreSetActors() { return BoolValue(PropOffsets_ConCamera.ignoreSetActors); }
	UActor*& lastFirstActor() { return Value<UActor*>(PropOffsets_ConCamera.lastFirstActor); }
	vec3& lastLocation() { return Value<vec3>(PropOffsets_ConCamera.lastLocation); }
	Rotator& lastRotation() { return Value<Rotator>(PropOffsets_ConCamera.lastRotation); }
	UActor*& lastSecondActor() { return Value<UActor*>(PropOffsets_ConCamera.lastSecondActor); }
	UActor*& secondActor() { return Value<UActor*>(PropOffsets_ConCamera.secondActor); }
	int& setActorCount() { return Value<int>(PropOffsets_ConCamera.setActorCount); }
};
