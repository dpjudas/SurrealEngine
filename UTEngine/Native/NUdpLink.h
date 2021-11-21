#pragma once

#include "UObject/UInternetLink.h"

class NUdpLink
{
public:
	static void RegisterFunctions();

	static void BindPort(UObject* Self, int* Port, bool* bUseNextAvailable, int& ReturnValue);
	static void ReadBinary(UObject* Self, IpAddr& Addr, int Count, uint8_t& B, int& ReturnValue);
	static void ReadText(UObject* Self, IpAddr& Addr, std::string& Str, int& ReturnValue);
	static void SendBinary(UObject* Self, const IpAddr& Addr, int Count, uint8_t B, bool& ReturnValue);
	static void SendText(UObject* Self, const IpAddr& Addr, const std::string& Str, bool& ReturnValue);
};
