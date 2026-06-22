#pragma once

#include "UObject/UObject.h"

class N227Projector
{
public:
    static void RegisterFunctions();

    static void AttachPrjDecal(UObject* Self);
    static void DeattachPrjDecal(UObject* Self);
    static void AttachActor(UObject* Self, UObject* OtherActor);
    static void DeattachActor(UObject* Self, UObject* OtherActor);
    static void DeattachAllActors(UObject* Self);
};