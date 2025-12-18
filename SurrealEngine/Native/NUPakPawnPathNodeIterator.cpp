#include "NUPakPawnPathNodeIterator.h"

#include "Utils/Logger.h"
#include "VM/NativeFunc.h"

void NUPakPawnPathNodeIterator::RegisterFunctions()
{
    RegisterVMNativeFunc_1("PawnPathNodeIterator", "SetPawn", &NUPakPawnPathNodeIterator::SetPawn, 0);
}

void NUPakPawnPathNodeIterator::SetPawn(UObject* Self, UObject*& Pawn)
{
    auto PPNISelf = UObject::Cast<UPakPawnPathNodeIterator>(Self);
    auto PPawn = UObject::Cast<UPawn>(Pawn);
    PPNISelf->SetPawn(PPawn);
}
