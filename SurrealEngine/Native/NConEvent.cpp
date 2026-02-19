#include "Precomp.h"
#include "NConEvent.h"
#include "VM/NativeFunc.h"
#include "Engine.h"

void NConEvent::RegisterFunctions()
{
	RegisterVMNativeFunc_2("ConEvent", "GetSoundLength", &NConEvent::GetSoundLength, 2054);
}

void NConEvent::GetSoundLength(UObject* Self, UObject* ASound, float& ReturnValue)
{
	LogUnimplemented("ConEvent.GetSoundLength");
	ReturnValue = 0.0f;
}
