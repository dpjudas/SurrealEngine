#pragma once

#include "UField.h"

class UEnum : public UField
{
public:
	using UField::UField;

	void Load(ObjectStream* stream) override;
	void Save(PackageStreamWriter* stream) override;

	Array<NameString> ElementNames;
};
