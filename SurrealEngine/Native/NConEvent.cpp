#include "Precomp.h"
#include "NConEvent.h"
#include "VM/NativeFunc.h"
#include "Packages/ConSys/Events/UConEvent.h"
#include "Packages/Engine/Resources/USound.h"

void NConEvent::RegisterFunctions()
{
	RegisterVMNativeFunc_2("ConEvent", "GetSoundLength", &NConEvent::GetSoundLength, 2054);
}

void NConEvent::GetSoundLength(UObject* Self, UObject* ASound, float& ReturnValue)
{
	auto SelfEvent = UObject::Cast<UConEvent>(Self);
	ReturnValue = SelfEvent->GetSoundLength(UObject::Cast<USound>(ASound));
}
