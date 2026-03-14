
#include "Precomp.h"
#include "UConSys.h"
#include "USound.h"
#include "Engine.h"

float UConEvent::GetSoundLength(USound* sound)
{
	return sound ? sound->GetDuration() : 0.0f;
}

/////////////////////////////////////////////////////////////////////////////

std::string UConEventRandomLabel::GetLabel(int labelIndex)
{
	LogUnimplemented("ConEventRandomLabel.GetLabel");
	return {};
}

int UConEventRandomLabel::GetLabelCount()
{
	LogUnimplemented("ConEventRandomLabel.GetLabelCount");
	return 0;
}

std::string UConEventRandomLabel::GetRandomLabel()
{
	LogUnimplemented("ConEventRandomLabel.GetRandomLabel");
	return {};
}

/////////////////////////////////////////////////////////////////////////////

void UConversation::BindActorEvents(UObject* actorToBind)
{
	LogUnimplemented("Conversation.BindActorEvents");
}

void UConversation::BindEvents(UObject* conBoundActors, UObject* invokeActor)
{
	LogUnimplemented("Conversation.BindEvents");
}

void UConversation::ClearBindEvents()
{
	LogUnimplemented("Conversation.ClearBindEvents");
}

UObject* UConversation::CreateConCamera()
{
	LogUnimplemented("Conversation.CreateConCamera");
	return nullptr;
}

UObject* UConversation::CreateFlagRef(const NameString& FlagName, bool flagValue)
{
	LogUnimplemented("Conversation.CreateFlagRef");
	return nullptr;
}

UObject* UConversation::GetSpeechAudio(int soundID)
{
	LogUnimplemented("Conversation.GetSpeechAudio");
	return nullptr;
}

float UConversation::GetSpeechLength(int soundID)
{
	LogUnimplemented("Conversation.GetSpeechLength");
	return 0.0f;
}
