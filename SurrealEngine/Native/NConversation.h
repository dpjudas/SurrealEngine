#pragma once

#include "UObject/UObject.h"

class NConversation
{
public:
	static void RegisterFunctions();

	static void BindActorEvents(UObject* Self, UObject* actorToBind);
	static void BindEvents(UObject* Self, UObject* conBoundActors, UObject* invokeActor);
	static void ClearBindEvents(UObject* Self);
	static void CreateConCamera(UObject* Self, UObject*& ReturnValue);
	static void CreateFlagRef(UObject* Self, const NameString& FlagName, bool flagValue, UObject*& ReturnValue);
	static void GetSpeechAudio(UObject* Self, int soundID, UObject*& ReturnValue);
	static void GetSpeechLength(UObject* Self, int soundID, float& ReturnValue);
};
