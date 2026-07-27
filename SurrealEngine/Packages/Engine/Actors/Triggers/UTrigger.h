#pragma once

#include "UTriggers.h"

class UTrigger : public UTriggers
{
public:
	using UTriggers::UTriggers;

	UClass*& ClassProximityType() { return Value<UClass*>(PropOffsets_Trigger.ClassProximityType); }
	float& DamageThreshold() { return Value<float>(PropOffsets_Trigger.DamageThreshold); }
	std::string& Message() { return Value<std::string>(PropOffsets_Trigger.Message); }
	float& ReTriggerDelay() { return Value<float>(PropOffsets_Trigger.ReTriggerDelay); }
	float& RepeatTriggerTime() { return Value<float>(PropOffsets_Trigger.RepeatTriggerTime); }
	UActor*& TriggerActor() { return Value<UActor*>(PropOffsets_Trigger.TriggerActor); }
	UActor*& TriggerActor2() { return Value<UActor*>(PropOffsets_Trigger.TriggerActor2); }
	float& TriggerTime() { return Value<float>(PropOffsets_Trigger.TriggerTime); }
	uint8_t& TriggerType() { return Value<uint8_t>(PropOffsets_Trigger.TriggerType); }
	BitfieldBool bInitiallyActive() { return BoolValue(PropOffsets_Trigger.bInitiallyActive); }
	BitfieldBool bTriggerOnceOnly() { return BoolValue(PropOffsets_Trigger.bTriggerOnceOnly); }
};
