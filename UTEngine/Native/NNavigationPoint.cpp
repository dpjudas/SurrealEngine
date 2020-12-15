
#include "Precomp.h"
#include "NNavigationPoint.h"
#include "VM/NativeFunc.h"

void NNavigationPoint::RegisterFunctions()
{
	RegisterVMNativeFunc_5("NavigationPoint", "describeSpec", &NNavigationPoint::describeSpec, 519);
}

void NNavigationPoint::describeSpec(UObject* Self, int iSpec, UObject*& Start, UObject*& End, int& ReachFlags, int& Distance)
{
	throw std::runtime_error("NavigationPoint.describeSpec not implemented");
}
