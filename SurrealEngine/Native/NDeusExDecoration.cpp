
#include "Precomp.h"
#include "NDeusExDecoration.h"
#include "VM/NativeFunc.h"
#include "UObject/UActor.h"
#include "UObject/ULevel.h"
#include "Engine.h"

void NDeusExDecoration::RegisterFunctions()
{
	RegisterVMNativeFunc_0("DeusExDecoration", "ConBindEvents", &NDeusExDecoration::ConBindEvents, 2101);
}

void NDeusExDecoration::ConBindEvents(UObject* Self)
{
	auto SelfDecoration = UObject::Cast<UDeusExDecoration>(Self);
	SelfDecoration->ConBindEvents();
}
