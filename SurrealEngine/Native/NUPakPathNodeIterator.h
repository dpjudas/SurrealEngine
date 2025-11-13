#pragma once

#include "UObject/UActor.h"

class NUPakPathNodeIterator
{
public:
    // TODO: Missing equivalent class in UObject folder (?)
    static void RegisterFunctions();

    static void BuildPath(UObject* Self, vec3* Start, vec3* End);
    static void CheckUPak(UObject* Self);
    static void GetFirst(UObject* Self, UObject*& ReturnValue);
    static void GetPrevious(UObject* Self, UObject*& ReturnValue);
    static void GetCurrent(UObject* Self, UObject*& ReturnValue);
    static void GetNext(UObject* Self, UObject*& ReturnValue);
    static void GetLast(UObject* Self, UObject*& ReturnValue);
    static void GetLastVisible(UObject* Self, UObject*& ReturnValue);
};