#pragma once

#include "UProperty.h"

class UFloatProperty : public UPropertyT<float>
{
public:
	UFloatProperty(NameString name, UClass* base, ObjectFlags flags);

	void LoadValue(void* data, ObjectStream* stream, const PropertyHeader& header) override;
	void LoadStructMemberValue(void* data, ObjectStream* stream) override;

	void SaveHeader(void* data, PropertyHeader& header) override;
	void SaveValue(void* data, PackageStreamWriter* stream) override;

	std::string PrintValue(const void* data) override;
	bool IsDefaultValue(void* val) override;
	void SetValueFromString(void* data, const std::string& valueString) override;
};
