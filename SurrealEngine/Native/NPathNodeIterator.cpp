#include "Precomp.h"
#include "NPathNodeIterator.h"
#include "VM/NativeFunc.h"
#include "Engine.h"

void NPathNodeIterator::RegisterFunctions()
{
	RegisterVMNativeFunc_2("PathNodeIterator", "BuildPath", &NPathNodeIterator::BuildPath, 0);
	RegisterVMNativeFunc_1("PathNodeIterator", "GetCurrent", &NPathNodeIterator::GetCurrent, 0);
	RegisterVMNativeFunc_1("PathNodeIterator", "GetFirst", &NPathNodeIterator::GetFirst, 0);
	RegisterVMNativeFunc_1("PathNodeIterator", "GetLast", &NPathNodeIterator::GetLast, 0);
	RegisterVMNativeFunc_1("PathNodeIterator", "GetLastVisible", &NPathNodeIterator::GetLastVisible, 0);
	RegisterVMNativeFunc_1("PathNodeIterator", "GetNext", &NPathNodeIterator::GetNext, 0);
	RegisterVMNativeFunc_1("PathNodeIterator", "GetPrevious", &NPathNodeIterator::GetPrevious, 0);
}

void NPathNodeIterator::BuildPath(UObject* Self, const vec3& Start, const vec3& End)
{
	LogUnimplemented("PathNodeIterator.BuildPath");
}

void NPathNodeIterator::GetCurrent(UObject* Self, UObject*& ReturnValue)
{
	LogUnimplemented("PathNodeIterator.GetCurrent");
	ReturnValue = nullptr;
}

void NPathNodeIterator::GetFirst(UObject* Self, UObject*& ReturnValue)
{
	LogUnimplemented("PathNodeIterator.GetFirst");
	ReturnValue = nullptr;
}

void NPathNodeIterator::GetLast(UObject* Self, UObject*& ReturnValue)
{
	LogUnimplemented("PathNodeIterator.GetLast");
	ReturnValue = nullptr;
}

void NPathNodeIterator::GetLastVisible(UObject* Self, UObject*& ReturnValue)
{
	LogUnimplemented("PathNodeIterator.GetLastVisible");
	ReturnValue = nullptr;
}

void NPathNodeIterator::GetNext(UObject* Self, UObject*& ReturnValue)
{
	LogUnimplemented("PathNodeIterator.GetNext");
	ReturnValue = nullptr;
}

void NPathNodeIterator::GetPrevious(UObject* Self, UObject*& ReturnValue)
{
	LogUnimplemented("PathNodeIterator.GetPrevious");
	ReturnValue = nullptr;
}
