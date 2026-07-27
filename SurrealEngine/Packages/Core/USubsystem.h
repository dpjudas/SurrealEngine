#pragma once

#include "UObject.h"

class USubsystem : public UObject
{
public:
	using UObject::UObject;

	virtual void LoadProperties(const NameString& from = "") {}
};
