
#include "Precomp.h"
#include "NWarpZoneInfo.h"
#include "VM/NativeFunc.h"

void NWarpZoneInfo::RegisterFunctions()
{
	RegisterVMNativeFunc_3("WarpZoneInfo", "UnWarp", &NWarpZoneInfo::UnWarp, 315);
	RegisterVMNativeFunc_3("WarpZoneInfo", "Warp", &NWarpZoneInfo::Warp, 314);
}

void NWarpZoneInfo::UnWarp(UObject* Self, vec3& Loc, vec3& Vel, Rotator& R)
{
	throw std::runtime_error("WarpZoneInfo.UnWarp not implemented");
}

void NWarpZoneInfo::Warp(UObject* Self, vec3& Loc, vec3& Vel, Rotator& R)
{
	throw std::runtime_error("WarpZoneInfo.Warp not implemented");
}
