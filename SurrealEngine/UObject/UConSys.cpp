
#include "Precomp.h"
#include "UConSys.h"
#include "USound.h"
#include "ULevel.h"
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

void UConversation::BindEvents(UObject** conBoundActors, UObject* invokeActor)
{
	// Note: conBoundActors seems to be an output parameter to be filled with actors we found in the events?
	// Not sure what invokeActor is used for. Also unclear how conOwnerName() and ownerRefCount() works.

	std::map<NameString, UActor*> nameToActor;

	for (UActor *actor : engine->Level->Actors)
	{
		if (actor)
		{
			NameString name = actor->BindName();
			if (!name.IsNone())
				nameToActor[name] = actor;
		}
	}

	if (auto actor = UObject::Cast<UActor>(invokeActor))
	{
		NameString name = actor->BindName();
		if (!name.IsNone())
			nameToActor[name] = actor;
	}

	for (UConEvent* e = eventList(); e; e = e->nextEvent())
	{
		switch ((EEventType)e->eventType())
		{
		default:
		case EEventType::Choice:
		case EEventType::SetFlag:
		case EEventType::CheckFlag:
		case EEventType::CheckObject:
		case EEventType::Random:
		case EEventType::Trigger:
		case EEventType::AddGoal:
		case EEventType::AddNote:
		case EEventType::AddSkillPoints:
		case EEventType::AddCredits:
		case EEventType::CheckPersona:
		case EEventType::Comment:
		case EEventType::End:
			break;
		case EEventType::Speech:
			if (auto speech = UObject::Cast<UConEventSpeech>(e))
			{
				speech->speaker() = nameToActor[speech->speakerName()];
				speech->speakingTo() = nameToActor[speech->speakingToName()];
			}
			break;
		case EEventType::TransferObject:
			if (auto transfer = UObject::Cast<UConEventTransferObject>(e))
			{
				transfer->fromActor() = nameToActor[transfer->fromName()];
				transfer->toActor() = nameToActor[transfer->toName()];
			}
			break;
		case EEventType::MoveCamera:
			if (auto moveCamera = UObject::Cast<UConEventMoveCamera>(e))
			{
				moveCamera->cameraActor() = nameToActor[moveCamera->cameraActorName()];
			}
			break;
		case EEventType::Animation:
			if (auto animation = UObject::Cast<UConEventAnimation>(e))
			{
				animation->eventOwner() = nameToActor[animation->eventOwnerName()];
			}
			break;
		case EEventType::Trade:
			if (auto trade = UObject::Cast<UConEventTrade>(e))
			{
				trade->eventOwner() = nameToActor[trade->eventOwnerName()];
			}
			break;
		case EEventType::Jump:
			if (auto jump = UObject::Cast<UConEventJump>(e))
			{
				// Do we need to do this?
				// jump->jumpCon() = jump->conID();
			}
			break;
		}
	}
}

void UConversation::ClearBindEvents()
{
	for (UConEvent* e = eventList(); e; e = e->nextEvent())
	{
		switch ((EEventType)e->eventType())
		{
		default:
		case EEventType::Choice:
		case EEventType::SetFlag:
		case EEventType::CheckFlag:
		case EEventType::CheckObject:
		case EEventType::Random:
		case EEventType::Trigger:
		case EEventType::AddGoal:
		case EEventType::AddNote:
		case EEventType::AddSkillPoints:
		case EEventType::AddCredits:
		case EEventType::CheckPersona:
		case EEventType::Comment:
		case EEventType::End:
			break;
		case EEventType::Speech:
			if (auto speech = UObject::Cast<UConEventSpeech>(e))
			{
				speech->speaker() = nullptr;
				speech->speakingTo() = nullptr;
			}
			break;
			break;
		case EEventType::TransferObject:
			if (auto transfer = UObject::Cast<UConEventTransferObject>(e))
			{
				transfer->fromActor() = nullptr;
				transfer->toActor() = nullptr;
			}
			break;
		case EEventType::MoveCamera:
			if (auto moveCamera = UObject::Cast<UConEventMoveCamera>(e))
			{
				moveCamera->cameraActor() = nullptr;
			}
			break;
		case EEventType::Animation:
			if (auto animation = UObject::Cast<UConEventAnimation>(e))
			{
				animation->eventOwner() = nullptr;
			}
			break;
		case EEventType::Trade:
			if (auto trade = UObject::Cast<UConEventTrade>(e))
			{
				trade->eventOwner() = nullptr;
			}
			break;
		case EEventType::Jump:
			if (auto jump = UObject::Cast<UConEventJump>(e))
			{
				// jump->jumpCon() = nullptr;
			}
			break;
		}
	}
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
