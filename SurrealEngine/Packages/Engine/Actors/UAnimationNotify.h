#pragma once

#include "Packages/Core/UObject.h"

class UActor;

// Unreal 227
struct sAnimNotify
{
	NameString AnimName;
	NameString FunctionName;
	int KeyFrame;
	eAnimNotifyEval NotifyEval;
	bool bCallOncePerLoop;
	bool bCalculatedFrame;
	bool bAlreadyCalled;
	int NumFrames;
	float CallKey;
};

class U227AnimationNotify : public UObject
{
public:
	using UObject::UObject;

	FixedArrayView<sAnimNotify, 255> AnimationNotify() { return FixedArray<sAnimNotify, 255>(PropOffsets_AnimationNotify.AnimationNotify); }
	int& NumNotifies() { return Value<int>(PropOffsets_AnimationNotify.NumNotifies); }
	UActor*& Owner() { return Value<UActor*>(PropOffsets_AnimationNotify.Owner); }
	BitfieldBool bInitialized() { return BoolValue(PropOffsets_AnimationNotify.bInitialized); }
	BitfieldBool bErrorOccured() { return BoolValue(PropOffsets_AnimationNotify.bErrorOccured); }
};
