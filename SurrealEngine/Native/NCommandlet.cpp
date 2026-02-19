
#include "Precomp.h"
#include "NCommandlet.h"
#include "VM/NativeFunc.h"
#include "Engine.h"

void NCommandlet::RegisterFunctions()
{
	RegisterVMNativeFunc_2("Commandlet", "Main", &NCommandlet::Main, 0);
}

void NCommandlet::Main(UObject* Self, const std::string& Parms, int& ReturnValue)
{
	LogUnimplemented("Commandlet.Main");
	ReturnValue = 0;
}
