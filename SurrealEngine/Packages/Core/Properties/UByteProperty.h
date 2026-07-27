#pragma once

#include "UProperty.h"

class UEnum;

class UByteProperty : public UPropertyT<uint8_t>
{
public:
	UByteProperty(NameString name, UClass* base, ObjectFlags flags);

	void Load(ObjectStream* stream) override;
	void Save(PackageStreamWriter* stream) override;

	void LoadValue(void* data, ObjectStream* stream, const PropertyHeader& header) override;
	void LoadStructMemberValue(void* data, ObjectStream* stream) override;

	void SaveHeader(void* data, PropertyHeader& header) override;
	void SaveValue(void* data, PackageStreamWriter* stream) override;

	std::string PrintValue(const void* data) override;
	bool IsDefaultValue(void* val) override;
	void SetValueFromString(void* data, const std::string& valueString) override;

	UEnum* EnumType = nullptr; // null if it is a normal byte, otherwise it is an enum type
};
