#include "Precomp.h"
#include "NConversation.h"
#include "VM/NativeFunc.h"
#include "UObject/UConSys.h"
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
	auto SelfConversation = UObject::Cast<UConversation>(Self);
	SelfConversation->BindActorEvents(actorToBind);
}

void NConversation::BindEvents(UObject* Self, UObject* conBoundActors, UObject* invokeActor)
{
	auto SelfConversation = UObject::Cast<UConversation>(Self);
	SelfConversation->BindEvents(conBoundActors, invokeActor);
}

void NConversation::ClearBindEvents(UObject* Self)
{
	auto SelfConversation = UObject::Cast<UConversation>(Self);
	SelfConversation->ClearBindEvents();
}

void NConversation::CreateConCamera(UObject* Self, UObject*& ReturnValue)
{
	auto SelfConversation = UObject::Cast<UConversation>(Self);
	ReturnValue = SelfConversation->CreateConCamera();
}

void NConversation::CreateFlagRef(UObject* Self, const NameString& FlagName, bool flagValue, UObject*& ReturnValue)
{
	auto SelfConversation = UObject::Cast<UConversation>(Self);
	ReturnValue = SelfConversation->CreateFlagRef(FlagName, flagValue);
}

void NConversation::GetSpeechAudio(UObject* Self, int soundID, UObject*& ReturnValue)
{
	auto SelfConversation = UObject::Cast<UConversation>(Self);
	ReturnValue = SelfConversation->GetSpeechAudio(soundID);
}

void NConversation::GetSpeechLength(UObject* Self, int soundID, float& ReturnValue)
{
	auto SelfConversation = UObject::Cast<UConversation>(Self);
	ReturnValue = SelfConversation->GetSpeechLength(soundID);
}
