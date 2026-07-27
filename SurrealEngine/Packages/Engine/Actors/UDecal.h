#pragma once

#include "UActor.h"

class BspNode;

class UDecal : public UActor
{
public:
	using UActor::UActor;

	UObject* AttachDecal(float traceDistance, vec3 decalDir);
	void DetachDecal();

	float& LastRenderedTime() { return Value<float>(PropOffsets_Decal.LastRenderedTime); }
	int& MultiDecalLevel() { return Value<int>(PropOffsets_Decal.MultiDecalLevel); }
	TypedScriptArray<void*> SurfList() { return DynamicArray<void*>(PropOffsets_Decal.SurfList); }

	Array<BspNode*> Nodes;
};
