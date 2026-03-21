
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
	// Called by ConPlayBase.StartConversation after it called BindEvents
	// It is only called if ownerRefCount() > 1 and there is an invoke actor
	// 
	// "Check to see if the conversation has multiple owners, in which case we
	// want to rebind all the events with this owner.  This allows conversations
	// to be shared by more than one owner."

	LogUnimplemented("Conversation.BindActorEvents");
}

void UConversation::BindEvents(UObject* conBoundActors, UObject* invokeActor)
{
	// Used when starting conversations.
	// Conversation.BindEvents(actors in conversation, start actor)

	LogUnimplemented("Conversation.BindEvents");
}

void UConversation::ClearBindEvents()
{
	// Called just before BindEvents to "clean up stuff"

	LogUnimplemented("Conversation.ClearBindEvents");
}

UObject* UConversation::CreateConCamera()
{
	LogUnimplemented("Conversation.CreateConCamera");
	return nullptr;
}

UObject* UConversation::CreateFlagRef(const NameString& FlagName, bool flagValue)
{
	LogUnimplemented("Conversation.CreateFlagRef(" + FlagName.ToString() + ", " + (flagValue ? "true" : "false") + ")");
	return nullptr;
}

UObject* UConversation::GetSpeechAudio(int soundID)
{
	LogUnimplemented("Conversation.GetSpeechAudio");
	//auto package = engine->packages->GetPackage("DeusExConAudio" + audioPackageName());
	return nullptr;
}

float UConversation::GetSpeechLength(int soundID)
{
	LogUnimplemented("Conversation.GetSpeechLength");
	return 0.0f;
}
