#include "Precomp.h"
#include "NTimeDemo.h"
#include "VM/NativeFunc.h"
#include "Engine.h"

void NTimeDemo::RegisterFunctions()
{
	RegisterVMNativeFunc_0("TimeDemo", "CloseFile", &NTimeDemo::CloseFile, 0);
	RegisterVMNativeFunc_0("TimeDemo", "OpenFile", &NTimeDemo::OpenFile, 0);
	RegisterVMNativeFunc_1("TimeDemo", "WriteToFile", &NTimeDemo::WriteToFile, 0);
}

void NTimeDemo::CloseFile(UObject* Self)
{
	LogUnimplemented("NTimeDemo::CloseFile not implemented");
}

void NTimeDemo::OpenFile(UObject* Self)
{
	LogUnimplemented("NTimeDemo::OpenFile not implemented");
}

void NTimeDemo::WriteToFile(UObject* Self, const std::string& Text)
{
	LogUnimplemented("NTimeDemo::WriteToFile not implemented");
}
