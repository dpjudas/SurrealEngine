#pragma once

#include "UObject/UActor.h"

class NUPakPawnPathNodeIterator
{
public:
    static void RegisterFunctions();

    static void SetPawn(UObject* Self, UObject*& Pawn);
};