#include "Precomp.h"
#include "NPawnPathNodeIterator.h"
#include "VM/NativeFunc.h"
#include "Engine.h"

void NPawnPathNodeIterator::RegisterFunctions()
{
	RegisterVMNativeFunc_1("PawnPathNodeIterator", "SetPawn", &NPawnPathNodeIterator::SetPawn, 0);
}

void NPawnPathNodeIterator::SetPawn(UObject* Self, UObject* P)
{
	Exception::Throw("NPawnPathNodeIterator::SetPawn not implemented");
}
