#pragma once

#include "UConEvent.h"

class UActor;

class UConEventMoveCamera : public UConEvent
{
public:
	using UConEvent::UConEvent;

	Rotator& Rotation() { return Value<Rotator>(PropOffsets_ConEventMoveCamera.Rotation); }
	UActor*& cameraActor() { return Value<UActor*>(PropOffsets_ConEventMoveCamera.cameraActor); }
	std::string& cameraActorName() { return Value<std::string>(PropOffsets_ConEventMoveCamera.cameraActorName); }
	vec3& cameraOffset() { return Value<vec3>(PropOffsets_ConEventMoveCamera.cameraOffset); }
	uint8_t& cameraPosition() { return Value<uint8_t>(PropOffsets_ConEventMoveCamera.cameraPosition); }
	uint8_t& cameraTransition() { return Value<uint8_t>(PropOffsets_ConEventMoveCamera.cameraTransition); }
	uint8_t& cameraType() { return Value<uint8_t>(PropOffsets_ConEventMoveCamera.cameraType); }
	float& centerModifier() { return Value<float>(PropOffsets_ConEventMoveCamera.centerModifier); }
	float& distanceMultiplier() { return Value<float>(PropOffsets_ConEventMoveCamera.distanceMultiplier); }
	float& heightModifier() { return Value<float>(PropOffsets_ConEventMoveCamera.heightModifier); }
};
