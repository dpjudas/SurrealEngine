
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

void UConAudioList::Load(ObjectStream* stream)
{
	UConObject::Load(stream);

	int count = stream->ReadIndex();
	for (int i = 0; i < count; i++)
	{
		conAudioList.push_back(stream->ReadObject<USound>());
	}
	stream->ThrowIfNotEnd();
}

/////////////////////////////////////////////////////////////////////////////

void UConEventRandomLabel::Load(ObjectStream* stream)
{
	UConEvent::Load(stream);
	int count = stream->ReadIndex();
	for (int i = 0; i < count; i++)
	{
		std::string label = stream->ReadString();
		labels.push_back(std::move(label));
	}
}

std::string UConEventRandomLabel::GetLabel(int labelIndex)
{
	std::string str = labels[labelIndex];
	return str;
}

int UConEventRandomLabel::GetLabelCount()
{
	return labels.size();
}

std::string UConEventRandomLabel::GetRandomLabel()
{
	int count = labels.size();
	if (labels.size() <= 0)
		return {};
	int index;
	if (cycleIndex() == count)
		bLabelsCycled() = true;

	if (!bCycleEvents() || (bCycleRandom() && bLabelsCycled()))
		index = std::rand() % count;
	else if (!bCycleOnce())
	{
		index = cycleIndex() % count;
		cycleIndex()++;
	}
	else 
	{
		index = cycleIndex() % count;
		cycleIndex()++;
	}

	return labels[index];

}

/////////////////////////////////////////////////////////////////////////////

void UConversation::BindActorEvents(UObject* actorToBind)
{
	// conevent internally uses invokeactor and actortobind. at least in this call, they're the one in the same.
	UActor* actorToBindAct = UObject::Cast<UActor>(actorToBind);
	if (!actorToBindAct)
		return;

	for (UConEvent* e = eventList(); e; e = e->nextEvent())
	{
		switch ((EEventType)e->eventType())
		{
		case EEventType::Speech:
			if (auto speech = UObject::Cast<UConEventSpeech>(e))
			{
				if (speech->speakerName() == actorToBindAct->BindName() || speech->speakerName() == actorToBindAct->BarkBindName())
					speech->speaker() = actorToBindAct;
				// internal note: maybe order of checks should be inverted
				if (speech->speakingToName() == actorToBindAct->BindName() || speech->speakingToName() == actorToBindAct->BarkBindName())
					speech->speakingTo() = actorToBindAct;
			}
			break;

		case EEventType::TransferObject:
			if (auto transfer = UObject::Cast<UConEventTransferObject>(e))
			{
				if ((transfer->fromName() == actorToBindAct->BindName() && bFirstPerson()) || (transfer->fromName() == actorToBindAct->BarkBindName()))
					transfer->fromActor() = actorToBindAct;

				if ((transfer->toName() == actorToBindAct->BindName() && bFirstPerson()) || (transfer->toName() == actorToBindAct->BarkBindName()))
					transfer->toActor() = actorToBindAct;
				transfer->giveObject() = engine->packages->FindClass("DeusEx.{}" + transfer->ObjectName());
			}
			break;
		/*not in original code*/
		case EEventType::MoveCamera:
			if (auto moveCamera = UObject::Cast<UConEventMoveCamera>(e))
			{
				if (moveCamera->cameraActorName() == actorToBindAct->BindName())
					moveCamera->cameraActor() = actorToBindAct;
			}
			break;
		
		case EEventType::Animation:
			if (auto animation = UObject::Cast<UConEventAnimation>(e))
			{
				bool bindActor = animation->eventOwnerName() == actorToBindAct->BindName();
				bool barkBindActor = animation->eventOwnerName() == actorToBindAct->BarkBindName();
				if ((bindActor && bFirstPerson()) || barkBindActor)
					animation->eventOwner() = actorToBindAct;	
			}
			break;

		case EEventType::Trade:
			if (auto trade = UObject::Cast<UConEventTrade>(e))
			{
				if (trade->eventOwnerName() == actorToBindAct->BindName())
					trade->eventOwner() = actorToBindAct;
			}
			break;
		
		case EEventType::CheckObject:
			if (auto eventCheckObject = UObject::Cast<UConEventCheckObject>(e))
			{
				if (eventCheckObject->ObjectName().starts_with("NK_"))
					eventCheckObject->checkObject() = nullptr;
				else
				{
					eventCheckObject->checkObject() = engine->packages->FindClass("DeusEx.{}" + eventCheckObject->ObjectName());
				}
			}

		default:
			break;
		}
	}

	//LogUnimplemented("Conversation.BindActorEvents");
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
	UClass* cls = engine->packages->FindClass("ConSys.ConFlagRef");
	NameString name;
	UObject* obj = engine->LevelPackage->NewObject(name, cls, ObjectFlags::Transient, true);
	UConFlagRef* flagObj = UObject::Cast<UConFlagRef>(obj);
	flagObj->FlagName() = FlagName;
	flagObj->Value() = flagValue;
	return obj;
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
