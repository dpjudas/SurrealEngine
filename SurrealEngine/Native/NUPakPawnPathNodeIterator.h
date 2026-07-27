#pragma once

#include "Packages/Core/UObject.h"

class NUPakPawnPathNodeIterator
{
public:
	static void RegisterFunctions();

	static void SetPawn(UObject* Self, UObject*& Pawn);
};