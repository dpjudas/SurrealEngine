#pragma once

#include "UBrush.h"

class USound;
class UTrigger;
class UPawn;
class UNavigationPoint;

class UMover : public UBrush
{
public:
	using UBrush::UBrush;

	vec3& BasePos() { return Value<vec3>(PropOffsets_Mover.BasePos); }
	Rotator& BaseRot() { return Value<Rotator>(PropOffsets_Mover.BaseRot); }
	uint8_t& BrushRaytraceKey() { return Value<uint8_t>(PropOffsets_Mover.BrushRaytraceKey); }
	NameString& BumpEvent() { return Value<NameString>(PropOffsets_Mover.BumpEvent); }
	uint8_t& BumpType() { return Value<uint8_t>(PropOffsets_Mover.BumpType); }
	int& ClientUpdate() { return Value<int>(PropOffsets_Mover.ClientUpdate); }
	USound*& ClosedSound() { return Value<USound*>(PropOffsets_Mover.ClosedSound); }
	USound*& ClosingSound() { return Value<USound*>(PropOffsets_Mover.ClosingSound); }
	float& DamageThreshold() { return Value<float>(PropOffsets_Mover.DamageThreshold); }
	float& DelayTime() { return Value<float>(PropOffsets_Mover.DelayTime); }
	int& EncroachDamage() { return Value<int>(PropOffsets_Mover.EncroachDamage); }
	UMover*& Follower() { return Value<UMover*>(PropOffsets_Mover.Follower); }
	uint8_t& KeyNum() { return Value<uint8_t>(PropOffsets_Mover.KeyNum); }
	FixedArrayView<vec3, 8> KeyPos() { return FixedArray<vec3, 8>(PropOffsets_Mover.KeyPos); }
	FixedArrayView<Rotator, 8> KeyRot() { return FixedArray<Rotator, 8>(PropOffsets_Mover.KeyRot); }
	UMover*& Leader() { return Value<UMover*>(PropOffsets_Mover.Leader); }
	USound*& MoveAmbientSound() { return Value<USound*>(PropOffsets_Mover.MoveAmbientSound); }
	float& MoveTime() { return Value<float>(PropOffsets_Mover.MoveTime); }
	uint8_t& MoverEncroachType() { return Value<uint8_t>(PropOffsets_Mover.MoverEncroachType); }
	uint8_t& MoverGlideType() { return Value<uint8_t>(PropOffsets_Mover.MoverGlideType); }
	uint8_t& NumKeys() { return Value<uint8_t>(PropOffsets_Mover.NumKeys); }
	vec3& OldPos() { return Value<vec3>(PropOffsets_Mover.OldPos); }
	vec3& OldPrePivot() { return Value<vec3>(PropOffsets_Mover.OldPrePivot); }
	Rotator& OldRot() { return Value<Rotator>(PropOffsets_Mover.OldRot); }
	USound*& OpenedSound() { return Value<USound*>(PropOffsets_Mover.OpenedSound); }
	USound*& OpeningSound() { return Value<USound*>(PropOffsets_Mover.OpeningSound); }
	float& OtherTime() { return Value<float>(PropOffsets_Mover.OtherTime); }
	NameString& PlayerBumpEvent() { return Value<NameString>(PropOffsets_Mover.PlayerBumpEvent); }
	uint8_t& PrevKeyNum() { return Value<uint8_t>(PropOffsets_Mover.PrevKeyNum); }
	vec3& RealPosition() { return Value<vec3>(PropOffsets_Mover.RealPosition); }
	Rotator& RealRotation() { return Value<Rotator>(PropOffsets_Mover.RealRotation); }
	UTrigger*& RecommendedTrigger() { return Value<UTrigger*>(PropOffsets_Mover.RecommendedTrigger); }
	NameString& ReturnGroup() { return Value<NameString>(PropOffsets_Mover.ReturnGroup); }
	vec3& SavedPos() { return Value<vec3>(PropOffsets_Mover.SavedPos); }
	Rotator& SavedRot() { return Value<Rotator>(PropOffsets_Mover.SavedRot); }
	UActor*& SavedTrigger() { return Value<UActor*>(PropOffsets_Mover.SavedTrigger); }
	vec3& SimInterpolate() { return Value<vec3>(PropOffsets_Mover.SimInterpolate); }
	vec3& SimOldPos() { return Value<vec3>(PropOffsets_Mover.SimOldPos); }
	int& SimOldRotPitch() { return Value<int>(PropOffsets_Mover.SimOldRotPitch); }
	int& SimOldRotRoll() { return Value<int>(PropOffsets_Mover.SimOldRotRoll); }
	int& SimOldRotYaw() { return Value<int>(PropOffsets_Mover.SimOldRotYaw); }
	float& StayOpenTime() { return Value<float>(PropOffsets_Mover.StayOpenTime); }
	UActor*& TriggerActor() { return Value<UActor*>(PropOffsets_Mover.TriggerActor); }
	UActor*& TriggerActor2() { return Value<UActor*>(PropOffsets_Mover.TriggerActor2); }
	UPawn*& WaitingPawn() { return Value<UPawn*>(PropOffsets_Mover.WaitingPawn); }
	uint8_t& WorldRaytraceKey() { return Value<uint8_t>(PropOffsets_Mover.WorldRaytraceKey); }
	BitfieldBool bClientPause() { return BoolValue(PropOffsets_Mover.bClientPause); }
	BitfieldBool bDamageTriggered() { return BoolValue(PropOffsets_Mover.bDamageTriggered); }
	BitfieldBool bDelaying() { return BoolValue(PropOffsets_Mover.bDelaying); }
	BitfieldBool bDynamicLightMover() { return BoolValue(PropOffsets_Mover.bDynamicLightMover); }
	BitfieldBool bOpening() { return BoolValue(PropOffsets_Mover.bOpening); }
	BitfieldBool bPlayerOnly() { return BoolValue(PropOffsets_Mover.bPlayerOnly); }
	BitfieldBool bSlave() { return BoolValue(PropOffsets_Mover.bSlave); }
	BitfieldBool bTriggerOnceOnly() { return BoolValue(PropOffsets_Mover.bTriggerOnceOnly); }
	BitfieldBool bUseTriggered() { return BoolValue(PropOffsets_Mover.bUseTriggered); }
	UNavigationPoint*& myMarker() { return Value<UNavigationPoint*>(PropOffsets_Mover.myMarker); }
	int& numTriggerEvents() { return Value<int>(PropOffsets_Mover.numTriggerEvents); }

	// LightSystem info
	struct
	{
		bool Calculated = false;
		vec3 Center = { 0.0f };
		float Radius = 0.0f;
		int MoverID = 0;
	} Light;
};
