#pragma once

#include "UConObject.h"

class USound;

class UConAudioList : public UConObject
{
public:
	using UConObject::UConObject;

	void Load(ObjectStream* stream) override;

	//DynamicArray& ConAudioList() { return Value<DynamicArray>(PropOffsets_ConAudioList.ConAudioList); }
	int& audioCount() { return Value<int>(PropOffsets_ConAudioList.audioCount); }

	Array<USound*> conAudioList;
};
