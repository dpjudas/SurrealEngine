
#include "Precomp.h"
#include "NZoneInfo.h"
#include "UObject/UActor.h"
#include "VM/NativeFunc.h"
#include "VM/Frame.h"

void NZoneInfo::RegisterFunctions()
{
	RegisterVMNativeFunc_2("ZoneInfo", "ZoneActors", &NZoneInfo::ZoneActors, 308);
}

void NZoneInfo::ZoneActors(UObject* Self, UObject* BaseClass, UObject*& Actor)
{
	Frame::CreatedIterator = std::make_unique<ZoneActorsIterator>(UObject::Cast<UZoneInfo>(Self), BaseClass, &Actor);
}
