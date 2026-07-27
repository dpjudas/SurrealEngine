#pragma once

#include "UField.h"

class UConst : public UField
{
public:
	using UField::UField;

	void Load(ObjectStream* stream) override;
	void Save(PackageStreamWriter* stream) override;

	std::string Constant;
};
