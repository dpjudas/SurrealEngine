#pragma once

#include "Packages/Core/UObject.h"

class UActor;

class ULevelBase : public UObject
{
public:
	using UObject::UObject;

	void Load(ObjectStream* stream) override;
	void Save(PackageStreamWriter* stream) override;

	Array<UActor*> Actors;

	std::string Protocol;
	std::string Host;
	int Port = 0;
	int Unknown = 0;
	std::string Map;
	Array<std::string> Options;
	std::string Portal;
};
