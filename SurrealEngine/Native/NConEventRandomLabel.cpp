#include "Precomp.h"
#include "NConEventRandomLabel.h"
#include "VM/NativeFunc.h"
#include "UObject/UConSys.h"
#include "Engine.h"

void NConEventRandomLabel::RegisterFunctions()
{
	RegisterVMNativeFunc_2("ConEventRandomLabel", "GetLabel", &NConEventRandomLabel::GetLabel, 2061);
	RegisterVMNativeFunc_1("ConEventRandomLabel", "GetLabelCount", &NConEventRandomLabel::GetLabelCount, 2060);
	RegisterVMNativeFunc_1("ConEventRandomLabel", "GetRandomLabel", &NConEventRandomLabel::GetRandomLabel, 2062);
}

void NConEventRandomLabel::GetLabel(UObject* Self, int labelIndex, std::string& ReturnValue)
{
	auto SelfLabel = UObject::Cast<UConEventRandomLabel>(Self);
	ReturnValue = SelfLabel->GetLabel(labelIndex);
}

void NConEventRandomLabel::GetLabelCount(UObject* Self, int& ReturnValue)
{
	auto SelfLabel = UObject::Cast<UConEventRandomLabel>(Self);
	ReturnValue = SelfLabel->GetLabelCount();
}

void NConEventRandomLabel::GetRandomLabel(UObject* Self, std::string& ReturnValue)
{
	auto SelfLabel = UObject::Cast<UConEventRandomLabel>(Self);
	ReturnValue = SelfLabel->GetRandomLabel();
}
