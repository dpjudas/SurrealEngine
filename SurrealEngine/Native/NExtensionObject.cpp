#include "Precomp.h"
#include "NExtensionObject.h"
#include "VM/NativeFunc.h"
#include "Engine.h"

void NExtensionObject::RegisterFunctions()
{
	RegisterVMNativeFunc_2("ExtensionObject", "StringToName", &NExtensionObject::StringToName, 1024);
}

void NExtensionObject::StringToName(UObject* Self, const std::string& str, NameString& ReturnValue)
{
	ReturnValue = NameString(str);
}
