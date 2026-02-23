
#include "Precomp.h"
#include "NWarpZoneInfo.h"
#include "VM/NativeFunc.h"
#include "Engine.h"

void NWarpZoneInfo::RegisterFunctions()
{
	RegisterVMNativeFunc_3("WarpZoneInfo", "UnWarp", &NWarpZoneInfo::UnWarp, 315);
	RegisterVMNativeFunc_3("WarpZoneInfo", "Warp", &NWarpZoneInfo::Warp, 314);
}

void NWarpZoneInfo::UnWarp(UObject* Self, vec3& Loc, vec3& Vel, Rotator& R)
{
	LogUnimplemented("WarpZoneInfo.UnWarp");
	auto WZISelf = UObject::Cast<UWarpZoneInfo>(Self);

	WZISelf->UnWarp(Loc, Vel, R);
}

void NWarpZoneInfo::Warp(UObject* Self, vec3& Loc, vec3& Vel, Rotator& R)
{
	LogUnimplemented("WarpZoneInfo.Warp");
	auto WZISelf = UObject::Cast<UWarpZoneInfo>(Self);

	WZISelf->Warp(Loc, Vel, R);
}
