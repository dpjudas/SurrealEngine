#pragma once

#include "UNetDriver.h"

class USurrealNetworkDevice : public UNetDriver
{
public:
	using UNetDriver::UNetDriver;

	std::string Class = "Engine.SurrealNetworkDevice";

	std::string GetPropertyAsString(const NameString& propertyName) const override;
	void SetPropertyFromString(const NameString& propertyName, const std::string& value) override;
};
