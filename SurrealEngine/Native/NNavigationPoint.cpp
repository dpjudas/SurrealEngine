
#include "Precomp.h"
#include "NNavigationPoint.h"
#include "VM/NativeFunc.h"
#include "UObject/UActor.h"
#include "UObject/ULevel.h"

void NNavigationPoint::RegisterFunctions()
{
	RegisterVMNativeFunc_5("NavigationPoint", "describeSpec", &NNavigationPoint::describeSpec, 519);
}

void NNavigationPoint::describeSpec(UObject* Self, int iSpec, UObject*& Start, UObject*& End, int& ReachFlags, int& Distance)
{
	UActor* SelfActor = UObject::Cast<UActor>(Self);
	ULevel* level = SelfActor->XLevel();
	if (iSpec >= 0 && (size_t)iSpec < level->ReachSpecs.size())
	{
		auto& spec = level->ReachSpecs[iSpec];
		Start = spec.startActor;
		End = spec.endActor;
		ReachFlags = spec.reachFlags;
		Distance = spec.distance;
	}
	else
	{
		Start = nullptr;
		End = nullptr;
		ReachFlags = 0;
		Distance = 0;
	}
}
