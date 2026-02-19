#include "Precomp.h"
#include "NConversation.h"
#include "VM/NativeFunc.h"
#include "Engine.h"

void NConversation::RegisterFunctions()
{
	RegisterVMNativeFunc_1("Conversation", "BindActorEvents", &NConversation::BindActorEvents, 2053);
	RegisterVMNativeFunc_2("Conversation", "BindEvents", &NConversation::BindEvents, 2052);
	RegisterVMNativeFunc_0("Conversation", "ClearBindEvents", &NConversation::ClearBindEvents, 2054);
	RegisterVMNativeFunc_1("Conversation", "CreateConCamera", &NConversation::CreateConCamera, 2051);
	RegisterVMNativeFunc_3("Conversation", "CreateFlagRef", &NConversation::CreateFlagRef, 2050);
	RegisterVMNativeFunc_2("Conversation", "GetSpeechAudio", &NConversation::GetSpeechAudio, 2055);
	RegisterVMNativeFunc_2("Conversation", "GetSpeechLength", &NConversation::GetSpeechLength, 2056);
}

void NConversation::BindActorEvents(UObject* Self, UObject* actorToBind)
{
	Exception::Throw("NConversation::BindActorEvents not implemented");
}

void NConversation::BindEvents(UObject* Self, UObject* conBoundActors, UObject* invokeActor)
{
	Exception::Throw("NConversation::BindEvents not implemented");
}

void NConversation::ClearBindEvents(UObject* Self)
{
	Exception::Throw("NConversation::ClearBindEvents not implemented");
}

void NConversation::CreateConCamera(UObject* Self, UObject*& ReturnValue)
{
	Exception::Throw("NConversation::CreateConCamera not implemented");
}

void NConversation::CreateFlagRef(UObject* Self, const NameString& FlagName, bool flagValue, UObject*& ReturnValue)
{
	Exception::Throw("NConversation::CreateFlagRef not implemented");
}

void NConversation::GetSpeechAudio(UObject* Self, int soundID, UObject*& ReturnValue)
{
	Exception::Throw("NConversation::GetSpeechAudio not implemented");
}

void NConversation::GetSpeechLength(UObject* Self, int soundID, float& ReturnValue)
{
	Exception::Throw("NConversation::GetSpeechLength not implemented");
}
