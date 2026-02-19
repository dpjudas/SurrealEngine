#pragma once

#include "UObject/UObject.h"

class NPawnPathNodeIterator
{
public:
	static void RegisterFunctions();

	static void SetPawn(UObject* Self, UObject* P);
};
