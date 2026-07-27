#pragma once

#include "UProperty.h"

class UStrProperty : public UPropertyT<std::string>
{
public:
	UStrProperty(NameString name, UClass* base, ObjectFlags flags);

	void LoadValue(void* data, ObjectStream* stream, const PropertyHeader& header) override;
	void LoadStructMemberValue(void* data, ObjectStream* stream) override;

	void SaveHeader(void* data, PropertyHeader& header) override;
	void SaveValue(void* data, PackageStreamWriter* stream) override;

	void SetValueFromString(void* data, const std::string& valueString) override;
	std::string PrintValue(const void* data) override;
	bool IsDefaultValue(void* val) override;
};
