#include "Precomp.h"
#include "NWebResponse.h"
#include "VM/NativeFunc.h"
#include "Engine.h"

void NWebResponse::RegisterFunctions()
{
	RegisterVMNativeFunc_0("WebResponse", "ClearSubst", &NWebResponse::ClearSubst, 0);
	RegisterVMNativeFunc_1("WebResponse", "IncludeBinaryFile", &NWebResponse::IncludeBinaryFile, 0);
	RegisterVMNativeFunc_1("WebResponse", "IncludeUHTM", &NWebResponse::IncludeUHTM, 0);
	RegisterVMNativeFunc_3("WebResponse", "Subst", &NWebResponse::Subst, 0);
}

void NWebResponse::ClearSubst(UObject* Self)
{
	LogUnimplemented("NWebResponse::ClearSubst not implemented");
}

void NWebResponse::IncludeBinaryFile(UObject* Self, const std::string& Filename)
{
	LogUnimplemented("NWebResponse::IncludeBinaryFile not implemented");
}

void NWebResponse::IncludeUHTM(UObject* Self, const std::string& Filename)
{
	LogUnimplemented("NWebResponse::IncludeUHTM not implemented");
}

void NWebResponse::Subst(UObject* Self, const std::string& Variable, const std::string& Value, BitfieldBool* bClear)
{
	LogUnimplemented("NWebResponse::Subst not implemented");
}
