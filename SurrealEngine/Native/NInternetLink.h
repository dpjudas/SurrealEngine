#pragma once

#include "UObject/UInternetLink.h"

class NInternetLink
{
public:
	static void RegisterFunctions();

	static void GetLastError(UObject* Self, int& ReturnValue);
	static void GetLocalIP(UObject* Self, IpAddr& Arg);
	static void IpAddrToString(UObject* Self, const IpAddr& Arg, std::string& ReturnValue);
	static void IsDataPending(UObject* Self, BitfieldBool& ReturnValue);
	static void ParseURL(UObject* Self, const std::string& URL, std::string& Addr, int& Port, std::string& LevelName, std::string& EntryName, BitfieldBool& ReturnValue);
	static void Resolve(UObject* Self, const std::string& Domain);
	static void StringToIpAddr(UObject* Self, const std::string& Str, IpAddr& Addr, BitfieldBool& ReturnValue);
	static void Validate(UObject* Self, const std::string& ValidationString, const std::string& GameName, std::string& ReturnValue);
};
