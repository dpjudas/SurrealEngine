#pragma once

#include "Packages/Engine/Actors/UActor.h"

class UBrush : public UActor
{
public:
	using UActor::UActor;

	Color& BrushColor() { return Value<Color>(PropOffsets_Brush.BrushColor); }
	uint8_t& CsgOper() { return Value<uint8_t>(PropOffsets_Brush.CsgOper); }
	Scale& MainScale() { return Value<Scale>(PropOffsets_Brush.MainScale); }
	int& PolyFlags() { return Value<int>(PropOffsets_Brush.PolyFlags); }
	vec3& PostPivot() { return Value<vec3>(PropOffsets_Brush.PostPivot); }
	Scale& PostScale() { return Value<Scale>(PropOffsets_Brush.PostScale); }
	Scale& TempScale() { return Value<Scale>(PropOffsets_Brush.TempScale); }
	UObject*& UnusedLightMesh() { return Value<UObject*>(PropOffsets_Brush.UnusedLightMesh); }
	BitfieldBool bColored() { return BoolValue(PropOffsets_Brush.bColored); }
};
