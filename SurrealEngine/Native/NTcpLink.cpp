
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

void NTcpLink::BindPort(UObject* Self, int* Port, BitfieldBool* bUseNextAvailable, int& ReturnValue)
{
	ReturnValue = UObject::Cast<UTcpLink>(Self)->BindPort(Port ? *Port : 7777, bUseNextAvailable ? *bUseNextAvailable : false);
}

void NTcpLink::Close(UObject* Self, BitfieldBool& ReturnValue)
{
	ReturnValue = UObject::Cast<UTcpLink>(Self)->Close();
}

void NTcpLink::IsConnected(UObject* Self, BitfieldBool& ReturnValue)
{
	ReturnValue = UObject::Cast<UTcpLink>(Self)->IsConnected();
}

void NTcpLink::Listen(UObject* Self, BitfieldBool& ReturnValue)
{
	ReturnValue = UObject::Cast<UTcpLink>(Self)->Listen();
}

void NTcpLink::Open(UObject* Self, const IpAddr& Addr, BitfieldBool& ReturnValue)
{
	ReturnValue = UObject::Cast<UTcpLink>(Self)->Open(Addr);
}

void NTcpLink::ReadBinary(UObject* Self, int Count, uint8_t& B, int& ReturnValue)
{
	ReturnValue = UObject::Cast<UTcpLink>(Self)->ReadBinary(Count, B);
}

void NTcpLink::ReadText(UObject* Self, std::string& Str, int& ReturnValue)
{
	ReturnValue = UObject::Cast<UTcpLink>(Self)->ReadText(Str);
}

void NTcpLink::SendBinary(UObject* Self, int Count, uint8_t B, int& ReturnValue)
{
	ReturnValue = UObject::Cast<UTcpLink>(Self)->SendBinary(Count, B);
}

void NTcpLink::SendText(UObject* Self, const std::string& Str, int& ReturnValue)
{
	ReturnValue = UObject::Cast<UTcpLink>(Self)->SendText(Str);
}
