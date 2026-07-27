#pragma once

#include "UProperty.h"

class UBoolProperty : public UProperty
{
public:
	UBoolProperty(NameString name, UClass* base, ObjectFlags flags);

	void LoadValue(void* data, ObjectStream* stream, const PropertyHeader& header) override;
	void LoadStructMemberValue(void* data, ObjectStream* stream) override;

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

	void SetValueFromString(void* data, const std::string& valueString) override;
	bool IsDefaultValue(void* val) override;
	std::string PrintValue(const void* data) override;

	GCAllocation* MarkPropertyElement(GCAllocation* marklist, void* data) override { return marklist; }

	bool GetBool(const void* data) const;
	void SetBool(void* data, bool value);
};
