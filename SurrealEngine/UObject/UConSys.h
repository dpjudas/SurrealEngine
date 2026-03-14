#pragma once

#include "UObject.h"
#include "UActor.h"

class USound;
class UConFlagRef;
class UConversation;
class UConSpeech;
class UConHistoryEvent;

class UConLight : public ULight
{
public:
	using ULight::ULight;
};

class UConObject : public UObject
{
public:
	using UObject::UObject;
};

class UConAudioList : public UConObject
{
public:
	using UConObject::UConObject;

	//DynamicArray& ConAudioList() { return Value<DynamicArray>(PropOffsets_ConAudioList.ConAudioList); }
	int& audioCount() { return Value<int>(PropOffsets_ConAudioList.audioCount); }
};

class UConCamera : public UConObject
{
public:
	using UConObject::UConObject;

	Rotator& Rotation() { return Value<Rotator>(PropOffsets_ConCamera.Rotation); }
	BitfieldBool bCameraLocationSaved() { return BoolValue(PropOffsets_ConCamera.bCameraLocationSaved); }
	BitfieldBool bDebug() { return BoolValue(PropOffsets_ConCamera.bDebug); }
	BitfieldBool bInteractiveCamera() { return BoolValue(PropOffsets_ConCamera.bInteractiveCamera); }
	BitfieldBool bUsingFallback() { return BoolValue(PropOffsets_ConCamera.bUsingFallback); }
	UActor*& cameraActor() { return Value<UActor*>(PropOffsets_ConCamera.cameraActor); }
	uint8_t& cameraFallbackPositions() { return Value<uint8_t>(PropOffsets_ConCamera.cameraFallbackPositions); }
	uint8_t& cameraHeightPositions() { return Value<uint8_t>(PropOffsets_ConCamera.cameraHeightPositions); }
	uint8_t& cameraMode() { return Value<uint8_t>(PropOffsets_ConCamera.cameraMode); }
	vec3& cameraOffset() { return Value<vec3>(PropOffsets_ConCamera.cameraOffset); }
	uint8_t& cameraPosition() { return Value<uint8_t>(PropOffsets_ConCamera.cameraPosition); }
	uint8_t& cameraType() { return Value<uint8_t>(PropOffsets_ConCamera.cameraType); }
	float& centerModifier() { return Value<float>(PropOffsets_ConCamera.centerModifier); }
	UConLight*& conLightSpeaker() { return Value<UConLight*>(PropOffsets_ConCamera.conLightSpeaker); }
	UConLight*& conLightSpeakingTo() { return Value<UConLight*>(PropOffsets_ConCamera.conLightSpeakingTo); }
	float& cosAngle() { return Value<float>(PropOffsets_ConCamera.cosAngle); }
	int& currentFallback() { return Value<int>(PropOffsets_ConCamera.currentFallback); }
	float& distanceMultiplier() { return Value<float>(PropOffsets_ConCamera.distanceMultiplier); }
	UActor*& firstActor() { return Value<UActor*>(PropOffsets_ConCamera.firstActor); }
	int& firstActorRotation() { return Value<int>(PropOffsets_ConCamera.firstActorRotation); }
	float& heightFallbackTrigger() { return Value<float>(PropOffsets_ConCamera.heightFallbackTrigger); }
	float& heightModifier() { return Value<float>(PropOffsets_ConCamera.heightModifier); }
	BitfieldBool ignoreSetActors() { return BoolValue(PropOffsets_ConCamera.ignoreSetActors); }
	UActor*& lastFirstActor() { return Value<UActor*>(PropOffsets_ConCamera.lastFirstActor); }
	vec3& lastLocation() { return Value<vec3>(PropOffsets_ConCamera.lastLocation); }
	Rotator& lastRotation() { return Value<Rotator>(PropOffsets_ConCamera.lastRotation); }
	UActor*& lastSecondActor() { return Value<UActor*>(PropOffsets_ConCamera.lastSecondActor); }
	UActor*& secondActor() { return Value<UActor*>(PropOffsets_ConCamera.secondActor); }
	int& setActorCount() { return Value<int>(PropOffsets_ConCamera.setActorCount); }
};

class UConChoice : public UConObject
{
public:
	using UConObject::UConObject;

	BitfieldBool bDisplayAsSpeech() { return BoolValue(PropOffsets_ConChoice.bDisplayAsSpeech); }
	std::string& choiceLabel() { return Value<std::string>(PropOffsets_ConChoice.choiceLabel); }
	std::string& choiceText() { return Value<std::string>(PropOffsets_ConChoice.choiceText); }
	UConFlagRef*& flagRef() { return Value<UConFlagRef*>(PropOffsets_ConChoice.flagRef); }
	UConChoice*& nextChoice() { return Value<UConChoice*>(PropOffsets_ConChoice.nextChoice); }
	int& skillLevelNeeded() { return Value<int>(PropOffsets_ConChoice.skillLevelNeeded); }
	UClass*& skillNeeded() { return Value<UClass*>(PropOffsets_ConChoice.skillNeeded); }
	int& soundID() { return Value<int>(PropOffsets_ConChoice.soundID); }
};

class UConEvent : public UConObject
{
public:
	using UConObject::UConObject;

	float GetSoundLength(USound* sound);

	UConversation*& Conversation() { return Value<UConversation*>(PropOffsets_ConEvent.Conversation); }
	std::string& Label() { return Value<std::string>(PropOffsets_ConEvent.Label); }
	uint8_t& eventType() { return Value<uint8_t>(PropOffsets_ConEvent.eventType); }
	UConEvent*& nextEvent() { return Value<UConEvent*>(PropOffsets_ConEvent.nextEvent); }
};

class UConEventAddCredits : public UConEvent
{
public:
	using UConEvent::UConEvent;

	int& creditsToAdd() { return Value<int>(PropOffsets_ConEventAddCredits.creditsToAdd); }
};

class UConEventAddGoal : public UConEvent
{
public:
	using UConEvent::UConEvent;

	BitfieldBool bGoalCompleted() { return BoolValue(PropOffsets_ConEventAddGoal.bGoalCompleted); }
	BitfieldBool bPrimaryGoal() { return BoolValue(PropOffsets_ConEventAddGoal.bPrimaryGoal); }
	NameString& goalName() { return Value<NameString>(PropOffsets_ConEventAddGoal.goalName); }
	std::string& goalText() { return Value<std::string>(PropOffsets_ConEventAddGoal.goalText); }
};

class UConEventAddNote : public UConEvent
{
public:
	using UConEvent::UConEvent;

	BitfieldBool bNoteAdded() { return BoolValue(PropOffsets_ConEventAddNote.bNoteAdded); }
	std::string& noteText() { return Value<std::string>(PropOffsets_ConEventAddNote.noteText); }
};

class UConEventAddSkillPoints : public UConEvent
{
public:
	using UConEvent::UConEvent;

	std::string& awardMessage() { return Value<std::string>(PropOffsets_ConEventAddSkillPoints.awardMessage); }
	int& pointsToAdd() { return Value<int>(PropOffsets_ConEventAddSkillPoints.pointsToAdd); }
};

class UConEventAnimation : public UConEvent
{
public:
	using UConEvent::UConEvent;

	NameString& Sequence() { return Value<NameString>(PropOffsets_ConEventAnimation.Sequence); }
	BitfieldBool bFinishAnim() { return BoolValue(PropOffsets_ConEventAnimation.bFinishAnim); }
	BitfieldBool bLoopAnim() { return BoolValue(PropOffsets_ConEventAnimation.bLoopAnim); }
	UActor*& eventOwner() { return Value<UActor*>(PropOffsets_ConEventAnimation.eventOwner); }
	std::string& eventOwnerName() { return Value<std::string>(PropOffsets_ConEventAnimation.eventOwnerName); }
};

class UConEventCheckFlag : public UConEvent
{
public:
	using UConEvent::UConEvent;

	UConFlagRef*& flagRef() { return Value<UConFlagRef*>(PropOffsets_ConEventCheckFlag.flagRef); }
	std::string& setLabel() { return Value<std::string>(PropOffsets_ConEventCheckFlag.setLabel); }
};

class UConEventCheckObject : public UConEvent
{
public:
	using UConEvent::UConEvent;

	std::string& ObjectName() { return Value<std::string>(PropOffsets_ConEventCheckObject.ObjectName); }
	UClass*& checkObject() { return Value<UClass*>(PropOffsets_ConEventCheckObject.checkObject); }
	std::string& failLabel() { return Value<std::string>(PropOffsets_ConEventCheckObject.failLabel); }
};

class UConEventCheckPersona : public UConEvent
{
public:
	using UConEvent::UConEvent;

	int& Value() { return UObject::Value<int>(PropOffsets_ConEventCheckPersona.Value); }
	uint8_t& condition() { return UObject::Value<uint8_t>(PropOffsets_ConEventCheckPersona.condition); }
	std::string& jumpLabel() { return UObject::Value<std::string>(PropOffsets_ConEventCheckPersona.jumpLabel); }
	uint8_t& personaType() { return UObject::Value<uint8_t>(PropOffsets_ConEventCheckPersona.personaType); }
};

class UConEventChoice : public UConEvent
{
public:
	using UConEvent::UConEvent;

	UConChoice*& ChoiceList() { return Value<UConChoice*>(PropOffsets_ConEventChoice.ChoiceList); }
	BitfieldBool bClearScreen() { return BoolValue(PropOffsets_ConEventChoice.bClearScreen); }
};

class UConEventComment : public UConEvent
{
public:
	using UConEvent::UConEvent;

	std::string& commentText() { return Value<std::string>(PropOffsets_ConEventComment.commentText); }
};

class UConEventEnd : public UConEvent
{
public:
	using UConEvent::UConEvent;
};

class UConEventJump : public UConEvent
{
public:
	using UConEvent::UConEvent;

	int& conID() { return Value<int>(PropOffsets_ConEventJump.conID); }
	UConversation*& jumpCon() { return Value<UConversation*>(PropOffsets_ConEventJump.jumpCon); }
	std::string& jumpLabel() { return Value<std::string>(PropOffsets_ConEventJump.jumpLabel); }
};

class UConEventMoveCamera : public UConEvent
{
public:
	using UConEvent::UConEvent;

	Rotator& Rotation() { return Value<Rotator>(PropOffsets_ConEventMoveCamera.Rotation); }
	UActor*& cameraActor() { return Value<UActor*>(PropOffsets_ConEventMoveCamera.cameraActor); }
	std::string& cameraActorName() { return Value<std::string>(PropOffsets_ConEventMoveCamera.cameraActorName); }
	vec3& cameraOffset() { return Value<vec3>(PropOffsets_ConEventMoveCamera.cameraOffset); }
	uint8_t& cameraPosition() { return Value<uint8_t>(PropOffsets_ConEventMoveCamera.cameraPosition); }
	uint8_t& cameraTransition() { return Value<uint8_t>(PropOffsets_ConEventMoveCamera.cameraTransition); }
	uint8_t& cameraType() { return Value<uint8_t>(PropOffsets_ConEventMoveCamera.cameraType); }
	float& centerModifier() { return Value<float>(PropOffsets_ConEventMoveCamera.centerModifier); }
	float& distanceMultiplier() { return Value<float>(PropOffsets_ConEventMoveCamera.distanceMultiplier); }
	float& heightModifier() { return Value<float>(PropOffsets_ConEventMoveCamera.heightModifier); }
};

class UConEventRandomLabel : public UConEvent
{
public:
	using UConEvent::UConEvent;

	std::string GetLabel(int labelIndex);
	int GetLabelCount();
	std::string GetRandomLabel();

	BitfieldBool bCycleEvents() { return BoolValue(PropOffsets_ConEventRandomLabel.bCycleEvents); }
	BitfieldBool bCycleOnce() { return BoolValue(PropOffsets_ConEventRandomLabel.bCycleOnce); }
	BitfieldBool bCycleRandom() { return BoolValue(PropOffsets_ConEventRandomLabel.bCycleRandom); }
	BitfieldBool bLabelsCycled() { return BoolValue(PropOffsets_ConEventRandomLabel.bLabelsCycled); }
	int& cycleIndex() { return Value<int>(PropOffsets_ConEventRandomLabel.cycleIndex); }
	//DynamicArray& labels() { return Value<DynamicArray>(PropOffsets_ConEventRandomLabel.labels); }
};

class UConEventSetFlag : public UConEvent
{
public:
	using UConEvent::UConEvent;

	UConFlagRef*& flagRef() { return Value<UConFlagRef*>(PropOffsets_ConEventSetFlag.flagRef); }
};

class UConEventSpeech : public UConEvent
{
public:
	using UConEvent::UConEvent;

	UConSpeech*& ConSpeech() { return Value<UConSpeech*>(PropOffsets_ConEventSpeech.ConSpeech); }
	BitfieldBool bBold() { return BoolValue(PropOffsets_ConEventSpeech.bBold); }
	BitfieldBool bContinued() { return BoolValue(PropOffsets_ConEventSpeech.bContinued); }
	UActor*& speaker() { return Value<UActor*>(PropOffsets_ConEventSpeech.speaker); }
	std::string& speakerName() { return Value<std::string>(PropOffsets_ConEventSpeech.speakerName); }
	UActor*& speakingTo() { return Value<UActor*>(PropOffsets_ConEventSpeech.speakingTo); }
	std::string& speakingToName() { return Value<std::string>(PropOffsets_ConEventSpeech.speakingToName); }
	uint8_t& speechFont() { return Value<uint8_t>(PropOffsets_ConEventSpeech.speechFont); }
};

class UConEventTrade : public UConEvent
{
public:
	using UConEvent::UConEvent;

	UActor*& eventOwner() { return Value<UActor*>(PropOffsets_ConEventTrade.eventOwner); }
	std::string& eventOwnerName() { return Value<std::string>(PropOffsets_ConEventTrade.eventOwnerName); }
};

class UConEventTransferObject : public UConEvent
{
public:
	using UConEvent::UConEvent;

	std::string& ObjectName() { return Value<std::string>(PropOffsets_ConEventTransferObject.ObjectName); }
	std::string& failLabel() { return Value<std::string>(PropOffsets_ConEventTransferObject.failLabel); }
	UActor*& fromActor() { return Value<UActor*>(PropOffsets_ConEventTransferObject.fromActor); }
	std::string& fromName() { return Value<std::string>(PropOffsets_ConEventTransferObject.fromName); }
	UClass*& giveObject() { return Value<UClass*>(PropOffsets_ConEventTransferObject.giveObject); }
	UActor*& toActor() { return Value<UActor*>(PropOffsets_ConEventTransferObject.toActor); }
	std::string& toName() { return Value<std::string>(PropOffsets_ConEventTransferObject.toName); }
	int& transferCount() { return Value<int>(PropOffsets_ConEventTransferObject.transferCount); }
};

class UConEventTrigger : public UConEvent
{
public:
	using UConEvent::UConEvent;

	NameString& triggerTag() { return Value<NameString>(PropOffsets_ConEventTrigger.triggerTag); }
};

class UConFlagRef : public UConObject
{
public:
	using UConObject::UConObject;

	NameString& FlagName() { return UObject::Value<NameString>(PropOffsets_ConFlagRef.FlagName); }
	BitfieldBool Value() { return UObject::BoolValue(PropOffsets_ConFlagRef.Value); }
	int& expiration() { return UObject::Value<int>(PropOffsets_ConFlagRef.expiration); }
	UConFlagRef*& nextFlagRef() { return UObject::Value<UConFlagRef*>(PropOffsets_ConFlagRef.nextFlagRef); }
};

class UConHistory : public UConObject
{
public:
	using UConObject::UConObject;

	UConHistory*& Next() { return Value<UConHistory*>(PropOffsets_ConHistory.Next); }
	BitfieldBool bInfoLink() { return BoolValue(PropOffsets_ConHistory.bInfoLink); }
	std::string& conOwnerName() { return Value<std::string>(PropOffsets_ConHistory.conOwnerName); }
	UConHistoryEvent*& firstEvent() { return Value<UConHistoryEvent*>(PropOffsets_ConHistory.firstEvent); }
	UConHistoryEvent*& lastEvent() { return Value<UConHistoryEvent*>(PropOffsets_ConHistory.lastEvent); }
	std::string& strDescription() { return Value<std::string>(PropOffsets_ConHistory.strDescription); }
	std::string& strLocation() { return Value<std::string>(PropOffsets_ConHistory.strLocation); }
};

class UConHistoryEvent : public UConObject
{
public:
	using UConObject::UConObject;

	UConHistoryEvent*& Next() { return Value<UConHistoryEvent*>(PropOffsets_ConHistoryEvent.Next); }
	std::string& Speech() { return Value<std::string>(PropOffsets_ConHistoryEvent.Speech); }
	std::string& conSpeaker() { return Value<std::string>(PropOffsets_ConHistoryEvent.conSpeaker); }
	int& soundID() { return Value<int>(PropOffsets_ConHistoryEvent.soundID); }
};

class UConItem : public UConObject
{
public:
	using UConObject::UConObject;

	UConObject*& ConObject() { return Value<UConObject*>(PropOffsets_ConItem.ConObject); }
	UConItem*& Next() { return Value<UConItem*>(PropOffsets_ConItem.Next); }
};

class UConListItem : public UConObject
{
public:
	using UConObject::UConObject;

	UConListItem*& Next() { return Value<UConListItem*>(PropOffsets_ConListItem.Next); }
	UConversation*& con() { return Value<UConversation*>(PropOffsets_ConListItem.con); }
};

class UConSpeech : public UConObject
{
public:
	using UConObject::UConObject;

	std::string& Speech() { return Value<std::string>(PropOffsets_ConSpeech.Speech); }
	int& soundID() { return Value<int>(PropOffsets_ConSpeech.soundID); }
};

class UConversation : public UConObject
{
public:
	using UConObject::UConObject;

	void BindActorEvents(UObject* actorToBind);
	void BindEvents(UObject* conBoundActors, UObject* invokeActor);
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

class UConversationList : public UConObject
{
public:
	using UConObject::UConObject;

	UConItem*& conversations() { return Value<UConItem*>(PropOffsets_ConversationList.conversations); }
	std::string& missionDescription() { return Value<std::string>(PropOffsets_ConversationList.missionDescription); }
	int& missionNumber() { return Value<int>(PropOffsets_ConversationList.missionNumber); }
};

class UConversationMissionList : public UConObject
{
public:
	using UConObject::UConObject;

	UConItem*& missions() { return Value<UConItem*>(PropOffsets_ConversationMissionList.missions); }
};
