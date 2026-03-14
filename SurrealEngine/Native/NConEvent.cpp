#include "Precomp.h"
#include "NConEvent.h"
#include "VM/NativeFunc.h"
#include "UObject/UConSys.h"
#include "UObject/USound.h"
#include "Engine.h"

void NConEvent::RegisterFunctions()
{
	RegisterVMNativeFunc_2("ConEvent", "GetSoundLength", &NConEvent::GetSoundLength, 2054);
}

void NConEvent::GetSoundLength(UObject* Self, UObject* ASound, float& ReturnValue)
{
	auto SelfEvent = UObject::Cast<UConEvent>(Self);
	ReturnValue = SelfEvent->GetSoundLength(UObject::Cast<USound>(ASound));
}
