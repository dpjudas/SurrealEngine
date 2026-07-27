#pragma once

#include "Packages/Core/UObject.h"

class UPalette : public UObject
{
public:
	using UObject::UObject;

	void Load(ObjectStream* stream) override;
	void Save(PackageStreamWriter* stream) override;

	Array<uint32_t> Colors;

	//TypedScriptArray<void*> Colors() { return DynamicArray<void*>(PropOffsets_Palette.Colors); } // native

	uint8_t FindBestColor(const Color& color) const;
};
