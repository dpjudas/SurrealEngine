#pragma once

#include "UProperty.h"

class UObjectProperty : public UPropertyT<UObject*>
{
public:
	UObjectProperty(NameString name, UClass* base, ObjectFlags flags);

	void Load(ObjectStream* stream) override;
	void Save(PackageStreamWriter* stream) override;

	void LoadValue(void* data, ObjectStream* stream, const PropertyHeader& header) override;
	void LoadStructMemberValue(void* data, ObjectStream* stream) override;

	void SaveHeader(void* data, PropertyHeader& header) override;
	void SaveValue(void* data, PackageStreamWriter* stream) override;

	std::string PrintValue(const void* data) override;
	bool IsDefaultValue(void* val) override;
	void SetValueFromString(void* data, const std::string& valueString) override;

	GCAllocation* MarkPropertyElement(GCAllocation* marklist, void* data) override;

	UClass* ObjectClass = nullptr;
};
