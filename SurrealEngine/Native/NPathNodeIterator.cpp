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
	Exception::Throw("NPathNodeIterator::BuildPath not implemented");
}

void NPathNodeIterator::GetCurrent(UObject* Self, UObject*& ReturnValue)
{
	Exception::Throw("NPathNodeIterator::GetCurrent not implemented");
}

void NPathNodeIterator::GetFirst(UObject* Self, UObject*& ReturnValue)
{
	Exception::Throw("NPathNodeIterator::GetFirst not implemented");
}

void NPathNodeIterator::GetLast(UObject* Self, UObject*& ReturnValue)
{
	Exception::Throw("NPathNodeIterator::GetLast not implemented");
}

void NPathNodeIterator::GetLastVisible(UObject* Self, UObject*& ReturnValue)
{
	Exception::Throw("NPathNodeIterator::GetLastVisible not implemented");
}

void NPathNodeIterator::GetNext(UObject* Self, UObject*& ReturnValue)
{
	Exception::Throw("NPathNodeIterator::GetNext not implemented");
}

void NPathNodeIterator::GetPrevious(UObject* Self, UObject*& ReturnValue)
{
	Exception::Throw("NPathNodeIterator::GetPrevious not implemented");
}
