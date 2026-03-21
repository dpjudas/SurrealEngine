
#include "Precomp.h"
#include "UConSys.h"
#include "USound.h"
#include "Engine.h"
#include "Package/PackageManager.h"

float UConEvent::GetSoundLength(USound* sound)
{
	return sound ? sound->GetDuration() : 1.0f;
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
	UClass* cls = engine->packages->FindClass("ConSys.ConCamera");
	NameString name;
	return engine->LevelPackage->NewObject(name, cls, ObjectFlags::Transient, true);
}

UObject* UConversation::CreateFlagRef(const NameString& FlagName, bool flagValue)
{
	LogUnimplemented("Conversation.CreateFlagRef(" + FlagName.ToString() + ", " + (flagValue ? "true" : "false") + ")");
	return nullptr;
}

UObject* UConversation::GetSpeechAudio(int soundID)
{
	auto package = engine->packages->GetPackage("DeusExConAudio" + audioPackageName());
	std::string missionStr = std::to_string(engine->DeusExLevelInfo->MissionNumber());
	if (missionStr.size() == 1)
		missionStr = "0" + missionStr;
	missionStr = "Mission" + missionStr;

	std::string audiolistName = "ConAudioList_" + missionStr;
	auto audioList = UObject::Cast<UConAudioList>(package->GetUObject("ConAudioList", audiolistName));

	// To do: use the audio list to find the sound index?

	std::string soundName = "ConAudio" + missionStr + "_" + std::to_string(soundID);
	auto sound = UObject::Cast<USound>(package->GetUObject("Sound", soundName));
	return sound;
}

float UConversation::GetSpeechLength(int soundID)
{
	USound* sound = UObject::Cast<USound>(GetSpeechAudio(soundID));
	return sound ? sound->GetDuration() : 1.0f;
}
