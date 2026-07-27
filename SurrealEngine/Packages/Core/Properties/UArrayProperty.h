#pragma once

#include "UProperty.h"

class UArrayProperty : public UProperty
{
public:
	UArrayProperty(NameString name, UClass* base, ObjectFlags flags);

	void Load(ObjectStream* stream) override;
	void Save(PackageStreamWriter* stream) override;

	void LoadValue(void* data, ObjectStream* stream, const PropertyHeader& header) override;
	void LoadStructMemberValue(void* data, ObjectStream* stream);
	void SaveHeader(void* data, PropertyHeader& header) override;
	void SaveValue(void* data, PackageStreamWriter* stream) override;

	size_t ElementAlignment() override;
	size_t ElementSize() override;

	void ConstructElement(void* data) override;
	void CopyConstructElement(void* data, const void* src) override;
	void DestructElement(void* data) override;
	void CopyElement(void* data, const void* src) override;
	bool CompareElement(const void* v1, const void* v2) override;
	bool CompareLessElement(const void* v1, const void* v2) override;

	void GetExportText(std::string& buf, const std::string& whitespace, UObject* obj, UObject* defobj, int i) override;
	std::string PrintValue(const void* data) override;

	GCAllocation* MarkPropertyElement(GCAllocation* marklist, void* data) override;

	UProperty* Inner = nullptr;
};
