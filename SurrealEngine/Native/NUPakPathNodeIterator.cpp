#include "NUPakPathNodeIterator.h"

#include "Utils/Logger.h"
#include "VM/NativeFunc.h"

void NUPakPathNodeIterator::RegisterFunctions()
{
    RegisterVMNativeFunc_2("PathNodeIterator", "BuildPath", &NUPakPathNodeIterator::BuildPath, 0);
    RegisterVMNativeFunc_0("PathNodeIterator", "CheckUPak", &NUPakPathNodeIterator::CheckUPak, 0);
    RegisterVMNativeFunc_1("PathNodeIterator", "GetFirst", &NUPakPathNodeIterator::GetFirst, 0);
    RegisterVMNativeFunc_1("PathNodeIterator", "GetPrevious", &NUPakPathNodeIterator::GetPrevious, 0);
    RegisterVMNativeFunc_1("PathNodeIterator", "GetCurrent", &NUPakPathNodeIterator::GetCurrent, 0);
    RegisterVMNativeFunc_1("PathNodeIterator", "GetNext", &NUPakPathNodeIterator::GetNext, 0);
    RegisterVMNativeFunc_1("PathNodeIterator", "GetLast", &NUPakPathNodeIterator::GetLast, 0);
    RegisterVMNativeFunc_1("PathNodeIterator", "GetLastVisible", &NUPakPathNodeIterator::GetLastVisible, 0);
}

void NUPakPathNodeIterator::BuildPath(UObject* Self, vec3* Start, vec3* End)
{
    LogUnimplemented("PathNodeIterator.BuildPath()");
}

void NUPakPathNodeIterator::CheckUPak(UObject* Self)
{
    LogUnimplemented("PathNodeIterator.CheckUPak()");
}

void NUPakPathNodeIterator::GetFirst(UObject* Self, UObject*& ReturnValue)
{
    LogUnimplemented("PathNodeIterator.GetFirst()");
}

void NUPakPathNodeIterator::GetPrevious(UObject* Self, UObject*& ReturnValue)
{
    LogUnimplemented("PathNodeIterator.GetPrevious()");
}

void NUPakPathNodeIterator::GetCurrent(UObject* Self, UObject*& ReturnValue)
{
    LogUnimplemented("PathNodeIterator.GetCurrent()");
}

void NUPakPathNodeIterator::GetNext(UObject* Self, UObject*& ReturnValue)
{
    LogUnimplemented("PathNodeIterator.GetNext()");
}

void NUPakPathNodeIterator::GetLast(UObject* Self, UObject*& ReturnValue)
{
    LogUnimplemented("PathNodeIterator.GetLast()");
}

void NUPakPathNodeIterator::GetLastVisible(UObject* Self, UObject*& ReturnValue)
{
    LogUnimplemented("PathNodeIterator.GetLastVisible()");
}
