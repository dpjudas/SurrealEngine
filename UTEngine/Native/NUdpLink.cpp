
#include "Precomp.h"
#include "NUdpLink.h"
#include "VM/NativeFunc.h"

void NUdpLink::RegisterFunctions()
{
	RegisterVMNativeFunc_3("UdpLink", "BindPort", &NUdpLink::BindPort, 0);
	RegisterVMNativeFunc_4("UdpLink", "ReadBinary", &NUdpLink::ReadBinary, 0);
	RegisterVMNativeFunc_3("UdpLink", "ReadText", &NUdpLink::ReadText, 0);
	RegisterVMNativeFunc_4("UdpLink", "SendBinary", &NUdpLink::SendBinary, 0);
	RegisterVMNativeFunc_3("UdpLink", "SendText", &NUdpLink::SendText, 0);
}

void NUdpLink::BindPort(UObject* Self, int* Port, bool* bUseNextAvailable, int& ReturnValue)
{
	throw std::runtime_error("UdpLink.BindPort not implemented");
}

void NUdpLink::ReadBinary(UObject* Self, IpAddr& Addr, int Count, uint8_t& B, int& ReturnValue)
{
	throw std::runtime_error("UdpLink.ReadBinary not implemented");
}

void NUdpLink::ReadText(UObject* Self, IpAddr& Addr, std::string& Str, int& ReturnValue)
{
	throw std::runtime_error("UdpLink.ReadText not implemented");
}

void NUdpLink::SendBinary(UObject* Self, const IpAddr& Addr, int Count, uint8_t B, bool& ReturnValue)
{
	throw std::runtime_error("UdpLink.SendBinary not implemented");
}

void NUdpLink::SendText(UObject* Self, const IpAddr& Addr, const std::string& Str, bool& ReturnValue)
{
	throw std::runtime_error("UdpLink.SendText not implemented");
}
