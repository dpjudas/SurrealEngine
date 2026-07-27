#pragma once

#include "UProperty.h"

class UStringProperty : public UPropertyT<std::string>
{
public:
	UStringProperty(NameString name, UClass* base, ObjectFlags flags);

	void LoadValue(void* data, ObjectStream* stream, const PropertyHeader& header) override;

	void SaveHeader(void* data, PropertyHeader& header) override;
	void SaveValue(void* data, PackageStreamWriter* stream) override;

	void SetValueFromString(void* data, const std::string& valueString) override;
	std::string PrintValue(const void* data) override;
	bool IsDefaultValue(void* val) override;
};
