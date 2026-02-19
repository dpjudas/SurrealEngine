
#include "Precomp.h"
#include "NDeusExDecoration.h"
#include "VM/NativeFunc.h"
#include "UObject/UActor.h"
#include "Engine.h"

void NDeusExDecoration::RegisterFunctions()
{
	RegisterVMNativeFunc_0("DeusExDecoration", "ConBindEvents", &NDeusExDecoration::ConBindEvents, 2101);
}

void NDeusExDecoration::ConBindEvents(UObject* Self)
{
	LogUnimplemented("DeusExDecoration.ConBindEvents");
}
