#pragma once

#include "UInternetLink.h"

class UTcpLink : public UInternetLink
{
public:
	UTcpLink(NameString name, UClass* base, ObjectFlags flags);
	~UTcpLink();

	void Tick(float elapsed) override;

	int BindPort(int Port, bool bUseNextAvailable);
	bool Listen();

	bool Open(const IpAddr& Addr);
	bool Close();

	bool IsConnected();

	int ReadBinary(int Count, uint8_t& B);
	int SendBinary(int Count, uint8_t B);

	int ReadText(std::string& Str);
	int SendText(const std::string& Str);

	UClass*& AcceptClass() { return Value<UClass*>(PropOffsets_TcpLink.AcceptClass); }
	uint8_t& LinkState() { return Value<uint8_t>(PropOffsets_TcpLink.LinkState); }
	IpAddr& RemoteAddr() { return Value<IpAddr>(PropOffsets_TcpLink.RemoteAddr); }
	TypedScriptArray<void*> SendFIFO() { return DynamicArray<void*>(PropOffsets_TcpLink.SendFIFO); }

	socket_t handle = invalid_socket_value;
};
