
#include "Precomp.h"
#include "NDeusExSaveInfo.h"
#include "VM/NativeFunc.h"
#include "UObject/UActor.h"
#include "Engine.h"

void NDeusExSaveInfo::RegisterFunctions()
{
	RegisterVMNativeFunc_0("DeusExSaveInfo", "UpdateTimeStamp", &NDeusExSaveInfo::UpdateTimeStamp, 3075);
}

void NDeusExSaveInfo::UpdateTimeStamp(UObject* Self)
{
	Exception::Throw("DeusExSaveInfo.UpdateTimeStamp not implemented");
}
