#pragma once

#include "Packages/Engine/Actors/Pawn/UPawn.h"

struct UDXInitialAllianceInfo
{
	NameString AllianceName;
	float AllianceLevel;
	union
	{
		uint32_t bPermanent : 1;
		uint32_t flags;
	};
};

struct UDXInitialAllianceInfoEx
{
	NameString AllianceName;
	float AllianceLevel;
	float AllianceAgitation;
	union
	{
		uint32_t bPermanent : 1;
		uint32_t flags;
	};
};

class UScriptedPawn : public UPawn
{
public:
	using UPawn::UPawn;
	FixedArrayView<NameString, 4> Carcasses() { return FixedArray<NameString, 4>(PropOffsets_ScriptedPawn.Carcasses); }
	FixedArrayView<UDXInitialAllianceInfo, 8> InitialAlliances() { return FixedArray<UDXInitialAllianceInfo, 8>(PropOffsets_ScriptedPawn.InitialAlliances); }
	FixedArrayView<UDXInitialAllianceInfoEx, 16> AlliancesEx() { return FixedArray<UDXInitialAllianceInfoEx, 16>(PropOffsets_ScriptedPawn.AlliancesEx); }
	int& NumCarcasses() { return Value<int>(PropOffsets_ScriptedPawn.NumCarcasses); }
	BitfieldBool bLikesNeutral() { return BoolValue(PropOffsets_ScriptedPawn.bLikesNeutral); }
	BitfieldBool bReverseAlliances() { return BoolValue(PropOffsets_ScriptedPawn.bReverseAlliances); }
	void AddCarcass(const NameString& CarcassName);
	void ConBindEvents();
	uint8_t GetAllianceType(const NameString& AllianceName);
	uint8_t GetPawnAllianceType(UPawn* QueryPawn);
	bool HaveSeenCarcass(const NameString& CarcassName);
	bool IsValidEnemy(UPawn* TestEnemy, std::optional<bool> bCheckAlliance);
};

struct XAIParams
{
	UActor* BestActor;
	float Score;
	float Visibility;
	float Volume;
	float Smell;
};
