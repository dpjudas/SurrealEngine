#pragma once

#include "UActor.h"
#include <thread>
#include <mutex>

#ifdef WIN32
typedef SOCKET socket_t;
#define invalid_socket_value INVALID_SOCKET
#else
typedef int socket_t;
#define invalid_socket_value -1
#endif

enum ELinkMode
{
	MODE_Text,
	MODE_Line,
	MODE_Binary
};

enum EReceiveMode
{
	RMODE_Manual,
	RMODE_Event
};

class UInternetLink : public UInternetInfo
{
public:
	using UInternetInfo::UInternetInfo;
	~UInternetLink();

	void Tick(float elapsed) override;

	int GetLastError();
	IpAddr GetLocalIP();
	bool IsDataPending();
	void Resolve(const std::string& Domain);

	std::string IpAddrToString(const IpAddr& Arg);
	bool StringToIpAddr(const std::string& Str, IpAddr& Addr);

	int& DataPending() { return Value<int>(PropOffsets_InternetLink.DataPending); }
	uint8_t& LinkMode() { return Value<uint8_t>(PropOffsets_InternetLink.LinkMode); }
	int& Port() { return Value<int>(PropOffsets_InternetLink.Port); }
	int& PrivateResolveInfo() { return Value<int>(PropOffsets_InternetLink.PrivateResolveInfo); } // native
	uint8_t& ReceiveMode() { return Value<uint8_t>(PropOffsets_InternetLink.ReceiveMode); }
	int& RemoteSocket() { return Value<int>(PropOffsets_InternetLink.RemoteSocket); }
	int& Socket() { return Value<int>(PropOffsets_InternetLink.Socket); }

	std::thread Thread;
	std::mutex Mutex;
	int LastError = 0;
	IpAddr ResolvedAddr = { 0 };
	int ResolveStatus = 0;
	IpAddr LocalIP = { 0 };
};

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
	Array<void*>& SendFIFO() { return Value<Array<void*>>(PropOffsets_TcpLink.SendFIFO); }

	socket_t handle = invalid_socket_value;
};

class UUdpLink : public UInternetLink
{
public:
	UUdpLink(NameString name, UClass* base, ObjectFlags flags);
	~UUdpLink();

	void Tick(float elapsed) override;

	int BindPort(int Port, bool bUseNextAvailable);

	int ReadBinary(IpAddr& Addr, int Count, uint8_t& B);
	bool SendBinary(const IpAddr& Addr, int Count, uint8_t B);

	int ReadText(IpAddr& Addr, std::string& Str);
	bool SendText(const IpAddr& Addr, const std::string& Str);

	int& BroadcastAddr() { return Value<int>(PropOffsets_UdpLink.BroadcastAddr); }

	socket_t handle = invalid_socket_value;
};
