#pragma once

#include "UObject.h"

class UField : public UObject
{
public:
	using UObject::UObject;

	void Load(ObjectStream* stream) override;
	void Save(PackageStreamWriter* stream) override;

	UField* BaseField = nullptr;
	UField* Next = nullptr;
};
