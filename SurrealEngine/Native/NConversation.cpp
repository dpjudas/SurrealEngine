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
	LogUnimplemented("NConversation::BindActorEvents not implemented");
}

void NConversation::BindEvents(UObject* Self, UObject* conBoundActors, UObject* invokeActor)
{
	LogUnimplemented("NConversation::BindEvents not implemented");
}

void NConversation::ClearBindEvents(UObject* Self)
{
	LogUnimplemented("NConversation::ClearBindEvents not implemented");
}

void NConversation::CreateConCamera(UObject* Self, UObject*& ReturnValue)
{
	LogUnimplemented("NConversation::CreateConCamera not implemented");
	ReturnValue = nullptr;
}

void NConversation::CreateFlagRef(UObject* Self, const NameString& FlagName, bool flagValue, UObject*& ReturnValue)
{
	LogUnimplemented("NConversation::CreateFlagRef not implemented");
	ReturnValue = nullptr;
}

void NConversation::GetSpeechAudio(UObject* Self, int soundID, UObject*& ReturnValue)
{
	LogUnimplemented("NConversation::GetSpeechAudio not implemented");
	ReturnValue = nullptr;
}

void NConversation::GetSpeechLength(UObject* Self, int soundID, float& ReturnValue)
{
	LogUnimplemented("NConversation::GetSpeechLength not implemented");
	ReturnValue = 0.0f;
}
