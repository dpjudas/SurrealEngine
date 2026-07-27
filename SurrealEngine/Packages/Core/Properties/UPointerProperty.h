#pragma once

#include "UProperty.h"

class UPointerProperty : public UPropertyT<void*>
{
public:
	using UPropertyT<void*>::UPropertyT;

	void Load(ObjectStream* stream) override;
	void Save(PackageStreamWriter* stream) override;

	void LoadValue(void* data, ObjectStream* stream, const PropertyHeader& header) override;
	void LoadStructMemberValue(void* data, ObjectStream* stream) override;

	void SaveHeader(void* data, PropertyHeader& header) override;
	void SaveValue(void* data, PackageStreamWriter* stream) override;

	std::string PrintValue(const void* data) override { return "pointer"; }
};
