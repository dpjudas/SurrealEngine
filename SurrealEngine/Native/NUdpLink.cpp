
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

void NUdpLink::BindPort(UObject* Self, int* Port, BitfieldBool* bUseNextAvailable, int& ReturnValue)
{
	ReturnValue = UObject::Cast<UUdpLink>(Self)->BindPort(Port ? *Port : 7777, bUseNextAvailable ? *bUseNextAvailable : false);
}

void NUdpLink::ReadBinary(UObject* Self, IpAddr& Addr, int Count, uint8_t& B, int& ReturnValue)
{
	ReturnValue = UObject::Cast<UUdpLink>(Self)->ReadBinary(Addr, Count, B);
}

void NUdpLink::ReadText(UObject* Self, IpAddr& Addr, std::string& Str, int& ReturnValue)
{
	ReturnValue = UObject::Cast<UUdpLink>(Self)->ReadText(Addr, Str);
}

void NUdpLink::SendBinary(UObject* Self, const IpAddr& Addr, int Count, uint8_t B, BitfieldBool& ReturnValue)
{
	ReturnValue = UObject::Cast<UUdpLink>(Self)->SendBinary(Addr, Count, B);
}

void NUdpLink::SendText(UObject* Self, const IpAddr& Addr, const std::string& Str, BitfieldBool& ReturnValue)
{
	ReturnValue = UObject::Cast<UUdpLink>(Self)->SendText(Addr, Str);
}
