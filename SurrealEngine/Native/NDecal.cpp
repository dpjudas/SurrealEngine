
#include "Precomp.h"
#include "NDecal.h"
#include "VM/NativeFunc.h"
#include "UObject/UActor.h"
#include "Engine.h"

void NDecal::RegisterFunctions()
{
	RegisterVMNativeFunc_3("Decal", "AttachDecal", &NDecal::AttachDecal, 0);
	RegisterVMNativeFunc_0("Decal", "DetachDecal", &NDecal::DetachDecal, 0);
}

void NDecal::AttachDecal(UObject* Self, float TraceDistance, vec3* DecalDir, UObject*& ReturnValue)
{
	ReturnValue = UObject::TryCast<UDecal>(Self)->AttachDecal(TraceDistance, DecalDir ? *DecalDir : vec3(0.0f));
}

void NDecal::DetachDecal(UObject* Self)
{
	UObject::TryCast<UDecal>(Self)->DetachDecal();
}
