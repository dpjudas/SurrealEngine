#pragma once

#include "UProperty.h"

class UFixedArrayProperty : public UProperty
{
public:
	using UProperty::UProperty;

	void Load(ObjectStream* stream) override;
	void Save(PackageStreamWriter* stream) override;

	void LoadValue(void* data, ObjectStream* stream, const PropertyHeader& header) override;

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

	std::string PrintValue(const void* data) override;

	GCAllocation* MarkPropertyElement(GCAllocation* marklist, void* data) override;

	UProperty* Inner = nullptr;
	int Count = 0;
};
