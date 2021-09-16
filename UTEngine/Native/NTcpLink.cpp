
#include "Precomp.h"
#include "NTcpLink.h"
#include "VM/NativeFunc.h"

void NTcpLink::RegisterFunctions()
{
	RegisterVMNativeFunc_3("TcpLink", "BindPort", &NTcpLink::BindPort, 0);
	RegisterVMNativeFunc_1("TcpLink", "Close", &NTcpLink::Close, 0);
	RegisterVMNativeFunc_1("TcpLink", "IsConnected", &NTcpLink::IsConnected, 0);
	RegisterVMNativeFunc_1("TcpLink", "Listen", &NTcpLink::Listen, 0);
	RegisterVMNativeFunc_2("TcpLink", "Open", &NTcpLink::Open, 0);
	RegisterVMNativeFunc_3("TcpLink", "ReadBinary", &NTcpLink::ReadBinary, 0);
	RegisterVMNativeFunc_2("TcpLink", "ReadText", &NTcpLink::ReadText, 0);
	RegisterVMNativeFunc_3("TcpLink", "SendBinary", &NTcpLink::SendBinary, 0);
	RegisterVMNativeFunc_2("TcpLink", "SendText", &NTcpLink::SendText, 0);
}

void NTcpLink::BindPort(UObject* Self, int* Port, bool* bUseNextAvailable, int& ReturnValue)
{
	throw std::runtime_error("TcpLink.BindPort not implemented");
}

void NTcpLink::Close(UObject* Self, bool& ReturnValue)
{
	throw std::runtime_error("TcpLink.Close not implemented");
}

void NTcpLink::IsConnected(UObject* Self, bool& ReturnValue)
{
	throw std::runtime_error("TcpLink.IsConnected not implemented");
}

void NTcpLink::Listen(UObject* Self, bool& ReturnValue)
{
	throw std::runtime_error("TcpLink.Listen not implemented");
}

void NTcpLink::Open(UObject* Self, const IpAddr& Addr, bool& ReturnValue)
{
	throw std::runtime_error("TcpLink.Open not implemented");
}

void NTcpLink::ReadBinary(UObject* Self, int Count, uint8_t& B, int& ReturnValue)
{
	throw std::runtime_error("TcpLink.ReadBinary not implemented");
}

void NTcpLink::ReadText(UObject* Self, std::string& Str, int& ReturnValue)
{
	throw std::runtime_error("TcpLink.ReadText not implemented");
}

void NTcpLink::SendBinary(UObject* Self, int Count, uint8_t B, int& ReturnValue)
{
	throw std::runtime_error("TcpLink.SendBinary not implemented");
}

void NTcpLink::SendText(UObject* Self, const std::string& Str, int& ReturnValue)
{
	throw std::runtime_error("TcpLink.SendText not implemented");
}
