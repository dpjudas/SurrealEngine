#pragma once

#include "UObject/UInternetLink.h"

class NTcpLink
{
public:
	static void RegisterFunctions();

	static void BindPort(UObject* Self, int* Port, bool* bUseNextAvailable, int& ReturnValue);
	static void Close(UObject* Self, bool& ReturnValue);
	static void IsConnected(UObject* Self, bool& ReturnValue);
	static void Listen(UObject* Self, bool& ReturnValue);
	static void Open(UObject* Self, const IpAddr& Addr, bool& ReturnValue);
	static void ReadBinary(UObject* Self, int Count, uint8_t& B, int& ReturnValue);
	static void ReadText(UObject* Self, std::string& Str, int& ReturnValue);
	static void SendBinary(UObject* Self, int Count, uint8_t B, int& ReturnValue);
	static void SendText(UObject* Self, const std::string& Str, int& ReturnValue);
};
