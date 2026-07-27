#pragma once

#include "UProperty.h"

class UStruct;

class UStructProperty : public UProperty
{
public:
	UStructProperty(NameString name, UClass* base, ObjectFlags flags);

	void Load(ObjectStream* stream) override;
	void Save(PackageStreamWriter* stream) override;

	void LoadValue(void* data, ObjectStream* stream, const PropertyHeader& header) override;
	void LoadStructMemberValue(void* data, ObjectStream* stream) override;

	void SaveHeader(void* data, PropertyHeader& header) override;
	void SaveValue(void* data, PackageStreamWriter* stream) override;

	size_t ElementAlignment() override;
	size_t ElementSize() override;

	void ConstructElement(void* data) override;
	void CopyConstructElement(void* data, const void* src) override;
	void CopyElement(void* data, const void* src) override;
	void DestructElement(void* data) override;
	bool CompareElement(const void* v1, const void* v2) override;
	bool CompareLessElement(const void* v1, const void* v2) override;

	void GetExportText(std::string& buf, const std::string& whitespace, UObject* obj, UObject* defobj, int i) override;
	std::string PrintValue(const void* data) override;
	void SetValueFromString(void* data, const std::string& valueString) override;

	GCAllocation* MarkPropertyElement(GCAllocation* marklist, void* data) override;

	UStruct* Struct = nullptr;
};
