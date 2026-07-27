#pragma once

#include "UConObject.h"

class UConEvent;
class UConFlagRef;

class UConversation : public UConObject
{
public:
	using UConObject::UConObject;

	void BindActorEvents(UObject* actorToBind);
	void BindEvents(UObject** conBoundActors, UObject* invokeActor);
	void ClearBindEvents();
	UObject* CreateConCamera();
	UObject* CreateFlagRef(const NameString& FlagName, bool flagValue);
	UObject* GetSpeechAudio(int soundID);
	float GetSpeechLength(int soundID);

	std::string& CreatedBy() { return Value<std::string>(PropOffsets_Conversation.CreatedBy); }
	std::string& Description() { return Value<std::string>(PropOffsets_Conversation.Description); }
	std::string& audioPackageName() { return Value<std::string>(PropOffsets_Conversation.audioPackageName); }
	BitfieldBool bCanBeInterrupted() { return BoolValue(PropOffsets_Conversation.bCanBeInterrupted); }
	BitfieldBool bCannotBeInterrupted() { return BoolValue(PropOffsets_Conversation.bCannotBeInterrupted); }
	BitfieldBool bDataLinkCon() { return BoolValue(PropOffsets_Conversation.bDataLinkCon); }
	BitfieldBool bDisplayOnce() { return BoolValue(PropOffsets_Conversation.bDisplayOnce); }
	BitfieldBool bFirstPerson() { return BoolValue(PropOffsets_Conversation.bFirstPerson); }
	BitfieldBool bGenerateAudioNames() { return BoolValue(PropOffsets_Conversation.bGenerateAudioNames); }
	BitfieldBool bInvokeBump() { return BoolValue(PropOffsets_Conversation.bInvokeBump); }
	BitfieldBool bInvokeFrob() { return BoolValue(PropOffsets_Conversation.bInvokeFrob); }
	BitfieldBool bInvokeRadius() { return BoolValue(PropOffsets_Conversation.bInvokeRadius); }
	BitfieldBool bInvokeSight() { return BoolValue(PropOffsets_Conversation.bInvokeSight); }
	BitfieldBool bNonInteractive() { return BoolValue(PropOffsets_Conversation.bNonInteractive); }
	BitfieldBool bRandomCamera() { return BoolValue(PropOffsets_Conversation.bRandomCamera); }
	NameString& conName() { return Value<NameString>(PropOffsets_Conversation.conName); }
	std::string& conOwnerName() { return Value<std::string>(PropOffsets_Conversation.conOwnerName); }
	int& conversationID() { return Value<int>(PropOffsets_Conversation.conversationID); }
	UConEvent*& eventList() { return Value<UConEvent*>(PropOffsets_Conversation.eventList); }
	UConFlagRef*& flagRefList() { return Value<UConFlagRef*>(PropOffsets_Conversation.flagRefList); }
	float& lastPlayedTime() { return Value<float>(PropOffsets_Conversation.lastPlayedTime); }
	int& ownerRefCount() { return Value<int>(PropOffsets_Conversation.ownerRefCount); }
	int& radiusDistance() { return Value<int>(PropOffsets_Conversation.radiusDistance); }
};
