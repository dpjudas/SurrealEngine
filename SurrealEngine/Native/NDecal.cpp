
#include "Precomp.h"
#include "NDecal.h"
#include "VM/NativeFunc.h"
#include "UObject/UActor.h"
#include "Engine.h"

void NDecal::RegisterFunctions()
{
	if (engine->LaunchInfo.IsDeusEx())
		RegisterVMNativeFunc_3("Decal", "AttachDecal", &NDecal::AttachDecal_Deus, 0);
	else
		RegisterVMNativeFunc_3("Decal", "AttachDecal", &NDecal::AttachDecal, 0);
	RegisterVMNativeFunc_0("Decal", "DetachDecal", &NDecal::DetachDecal, 0);
}

void NDecal::AttachDecal(UObject* Self, float TraceDistance, vec3* DecalDir, UObject*& ReturnValue)
{
	ReturnValue = UObject::TryCast<UDecal>(Self)->AttachDecal(TraceDistance, DecalDir ? *DecalDir : vec3(0.0f));
}

void NDecal::AttachDecal_Deus(UObject* Self, float TraceDistance, vec3* DecalDir, BitfieldBool& ReturnValue)
{
	// Why does Deus Ex not need the decal object? Decals are no longer actors?
	LogUnimplemented("Decal.AttachDecal(Deus)");
	ReturnValue = true;
}

void NDecal::DetachDecal(UObject* Self)
{
	UObject::TryCast<UDecal>(Self)->DetachDecal();
}
