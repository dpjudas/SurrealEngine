
#include "Precomp.h"
#include "NDecal.h"
#include "VM/NativeFunc.h"
#include "Engine.h"

void NDecal::RegisterFunctions()
{
	RegisterVMNativeFunc_3("Decal", "AttachDecal", &NDecal::AttachDecal, 0);
	RegisterVMNativeFunc_0("Decal", "DetachDecal", &NDecal::DetachDecal, 0);
}

void NDecal::AttachDecal(UObject* Self, float TraceDistance, vec3* DecalDir, UObject*& ReturnValue)
{
	engine->LogUnimplemented("Decal.AttachDecal");
	ReturnValue = nullptr;
}

void NDecal::DetachDecal(UObject* Self)
{
	engine->LogUnimplemented("Decal.DetachDecal");
}
