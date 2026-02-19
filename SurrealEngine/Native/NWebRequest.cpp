#include "Precomp.h"
#include "NWebRequest.h"
#include "VM/NativeFunc.h"
#include "Engine.h"

void NWebRequest::RegisterFunctions()
{
	RegisterVMNativeFunc_2("WebRequest", "AddVariable", &NWebRequest::AddVariable, 0);
	RegisterVMNativeFunc_2("WebRequest", "DecodeBase64", &NWebRequest::DecodeBase64, 0);
	RegisterVMNativeFunc_3("WebRequest", "GetVariable", &NWebRequest::GetVariable, 0);
	RegisterVMNativeFunc_2("WebRequest", "GetVariableCount", &NWebRequest::GetVariableCount, 0);
	RegisterVMNativeFunc_4("WebRequest", "GetVariableNumber", &NWebRequest::GetVariableNumber, 0);
}

void NWebRequest::AddVariable(UObject* Self, const std::string& VariableName, const std::string& Value)
{
	LogUnimplemented("WebRequest.AddVariable");
}

void NWebRequest::DecodeBase64(UObject* Self, const std::string& Encoded, std::string& ReturnValue)
{
	LogUnimplemented("WebRequest.DecodeBase64");
	ReturnValue = "";
}

void NWebRequest::GetVariable(UObject* Self, const std::string& VariableName, std::string* DefaultValue, std::string& ReturnValue)
{
	LogUnimplemented("WebRequest.GetVariable");
	ReturnValue = "";
}

void NWebRequest::GetVariableCount(UObject* Self, const std::string& VariableName, int& ReturnValue)
{
	LogUnimplemented("WebRequest.GetVariableCount");
	ReturnValue = 0;
}

void NWebRequest::GetVariableNumber(UObject* Self, const std::string& VariableName, int Number, std::string* DefaultValue, std::string& ReturnValue)
{
	LogUnimplemented("WebRequest.GetVariableNumber");
	ReturnValue = "";
}
