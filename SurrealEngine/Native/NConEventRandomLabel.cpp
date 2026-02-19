#include "Precomp.h"
#include "NConEventRandomLabel.h"
#include "VM/NativeFunc.h"
#include "Engine.h"

void NConEventRandomLabel::RegisterFunctions()
{
	RegisterVMNativeFunc_2("ConEventRandomLabel", "GetLabel", &NConEventRandomLabel::GetLabel, 2061);
	RegisterVMNativeFunc_1("ConEventRandomLabel", "GetLabelCount", &NConEventRandomLabel::GetLabelCount, 2060);
	RegisterVMNativeFunc_1("ConEventRandomLabel", "GetRandomLabel", &NConEventRandomLabel::GetRandomLabel, 2062);
}

void NConEventRandomLabel::GetLabel(UObject* Self, int labelIndex, std::string& ReturnValue)
{
	LogUnimplemented("NConEventRandomLabel.GetLabel");
	ReturnValue = "";
}

void NConEventRandomLabel::GetLabelCount(UObject* Self, int& ReturnValue)
{
	LogUnimplemented("NConEventRandomLabel.GetLabelCount");
	ReturnValue = 0;
}

void NConEventRandomLabel::GetRandomLabel(UObject* Self, std::string& ReturnValue)
{
	LogUnimplemented("NConEventRandomLabel.GetRandomLabel");
	ReturnValue = "";
}
