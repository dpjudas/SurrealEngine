
#include "Precomp.h"
#include "NDecal.h"
#include "VM/NativeFunc.h"

void NDecal::RegisterFunctions()
{
	RegisterVMNativeFunc_3("Decal", "AttachDecal", &NDecal::AttachDecal, 0);
	RegisterVMNativeFunc_0("Decal", "DetachDecal", &NDecal::DetachDecal, 0);
}

void NDecal::AttachDecal(UObject* Self, float TraceDistance, vec3* DecalDir, UObject*& ReturnValue)
{
	throw std::runtime_error("Decal.AttachDecal not implemented");
}

void NDecal::DetachDecal(UObject* Self)
{
	throw std::runtime_error("Decal.DetachDecal not implemented");
}
