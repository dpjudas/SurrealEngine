#pragma once

#include "UObjectProperty.h"

class UClassProperty : public UObjectProperty
{
public:
	using UObjectProperty::UObjectProperty;

	void Load(ObjectStream* stream) override;
	void Save(PackageStreamWriter* stream) override;

	std::string PrintValue(const void* data) override;

	UClass* MetaClass = nullptr;
};
