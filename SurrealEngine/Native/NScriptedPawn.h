#pragma once

#include "UObject/UObject.h"

class NScriptedPawn
{
public:
	static void RegisterFunctions();

	static void AddCarcass(UObject* Self, const NameString& CarcassName);
	static void ConBindEvents(UObject* Self);
	static void GetAllianceType(UObject* Self, const NameString& AllianceName, uint8_t& ReturnValue);
	static void GetPawnAllianceType(UObject* Self, UObject* QueryPawn, uint8_t& ReturnValue);
	static void HaveSeenCarcass(UObject* Self, const NameString& CarcassName, BitfieldBool& ReturnValue);
	static void IsValidEnemy(UObject* Self, UObject* TestEnemy, BitfieldBool* bCheckAlliance, BitfieldBool& ReturnValue);
};
