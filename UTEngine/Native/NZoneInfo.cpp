
#include "Precomp.h"
#include "NZoneInfo.h"
#include "VM/NativeFunc.h"

void NZoneInfo::RegisterFunctions()
{
	RegisterVMNativeFunc_2("ZoneInfo", "ZoneActors", &NZoneInfo::ZoneActors, 308);
}

void NZoneInfo::ZoneActors(UObject* Self, UObject* BaseClass, UObject*& Actor)
{
	throw std::runtime_error("ZoneInfo.ZoneActors not implemented");
}
