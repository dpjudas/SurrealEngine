
#include "Precomp.h"
#include "NInternetLink.h"
#include "VM/NativeFunc.h"
#include "Engine.h"

void NInternetLink::RegisterFunctions()
{
	RegisterVMNativeFunc_1("InternetLink", "GetLastError", &NInternetLink::GetLastError, 0);
	RegisterVMNativeFunc_1("InternetLink", "GetLocalIP", &NInternetLink::GetLocalIP, 0);
	RegisterVMNativeFunc_2("InternetLink", "IpAddrToString", &NInternetLink::IpAddrToString, 0);
	RegisterVMNativeFunc_1("InternetLink", "IsDataPending", &NInternetLink::IsDataPending, 0);
	RegisterVMNativeFunc_6("InternetLink", "ParseURL", &NInternetLink::ParseURL, 0);
	RegisterVMNativeFunc_1("InternetLink", "Resolve", &NInternetLink::Resolve, 0);
	RegisterVMNativeFunc_3("InternetLink", "StringToIpAddr", &NInternetLink::StringToIpAddr, 0);
	RegisterVMNativeFunc_3("InternetLink", "Validate", &NInternetLink::Validate, 0);
}

void NInternetLink::GetLastError(UObject* Self, int& ReturnValue)
{
	ReturnValue = Self->Cast<UInternetLink>(Self)->GetLastError();
}

void NInternetLink::GetLocalIP(UObject* Self, IpAddr& Arg)
{
	Arg = Self->Cast<UInternetLink>(Self)->GetLocalIP();
}

void NInternetLink::IsDataPending(UObject* Self, BitfieldBool& ReturnValue)
{
	ReturnValue = Self->Cast<UInternetLink>(Self)->IsDataPending();
}

void NInternetLink::Resolve(UObject* Self, const std::string& Domain)
{
	Self->Cast<UInternetLink>(Self)->Resolve(Domain);
}

void NInternetLink::IpAddrToString(UObject* Self, const IpAddr& Arg, std::string& ReturnValue)
{
	ReturnValue = Self->Cast<UInternetLink>(Self)->IpAddrToString(Arg);
}

void NInternetLink::StringToIpAddr(UObject* Self, const std::string& Str, IpAddr& Addr, BitfieldBool& ReturnValue)
{
	ReturnValue = Self->Cast<UInternetLink>(Self)->StringToIpAddr(Str, Addr);
}

void NInternetLink::ParseURL(UObject* Self, const std::string& URL, std::string& Addr, int& Port, std::string& LevelName, std::string& EntryName, BitfieldBool& ReturnValue)
{
	throw std::runtime_error("InternetLink.ParseURL not implemented");
}

void NInternetLink::Validate(UObject* Self, const std::string& ValidationString, const std::string& GameName, std::string& ReturnValue)
{
	throw std::runtime_error("InternetLink.Validate not implemented");
}
