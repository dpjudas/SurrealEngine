#pragma once

#include "UProperty.h"

// Some 227 thing. Probably wrong implementation.
class UAnyProperty : public UProperty
{
public:
	using UProperty::UProperty;

	void Load(ObjectStream* stream) override;
	void Save(PackageStreamWriter* stream) override;

	size_t ElementAlignment() override { return 1; }
	size_t ElementSize() override { return 1; }

	void ConstructElement(void* data) {}
	void CopyConstructElement(void* data, const void* src) {}
	void CopyElement(void* data, const void* src) {}
	void DestructElement(void* data) {}
	bool CompareElement(const void* v1, const void* v2) { return true; }
	bool CompareLessElement(const void* v1, const void* v2) { return false; }
	GCAllocation* MarkPropertyElement(GCAllocation* marklist, void* data) override { return marklist; }
};
