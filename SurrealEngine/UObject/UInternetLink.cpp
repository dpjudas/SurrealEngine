
#include "Precomp.h"
#include "UInternetLink.h"
#include "VM/ScriptCall.h"
#include "VM/Frame.h"
#include "Package/PackageManager.h"
#include "Engine.h"

#ifdef WIN32
#include <WinSock2.h>
typedef unsigned long in_addr_t;
#pragma comment(lib, "ws2_32.lib")
#else
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
static int closesocket(int fd) { return close(fd); }
#endif

UInternetLink::~UInternetLink()
{
	if (Thread.joinable())
		Thread.join();
}

void UInternetLink::Tick(float elapsed, bool tickedFlag)
{
	UInternetInfo::Tick(elapsed, tickedFlag);

	std::unique_lock<std::mutex> lock(Mutex);
	IpAddr resolvedAddr = ResolvedAddr;
	int resolveStatus = ResolveStatus;
	if (resolveStatus > 1)
		ResolveStatus = 0;
	lock.unlock();

	if (ResolveStatus == 2)
	{
		UFunction* func = FindEventFunction(this, "Resolved");
		if (func)
		{
			UStructProperty prop({}, nullptr, ObjectFlags::NoFlags);
			prop.Struct = UObject::Cast<UStructProperty>(func->Properties[0])->Struct;
			CallEvent(this, EventName::Resolved, { ExpressionValue::Variable(&resolvedAddr, &prop) });
		}
	}
	else if (ResolveStatus == 3)
	{
		CallEvent(this, EventName::ResolveFailed);
	}
}

int UInternetLink::GetLastError()
{
	std::unique_lock<std::mutex> lock(Mutex);
	return LastError;
}

IpAddr UInternetLink::GetLocalIP()
{
	return LocalIP;
}

bool UInternetLink::IsDataPending()
{
	return DataPending() != 0;
}

void UInternetLink::Resolve(const std::string& Domain)
{
	std::unique_lock<std::mutex> lock(Mutex);
	if (ResolveStatus != 1)
	{
		ResolveStatus = 1;

		if (Thread.joinable())
			Thread.detach();

		std::string _address = Domain;
		auto threadMain = [=]()
		{
			in_addr_t ipv4_address = inet_addr(_address.c_str());
			if (ipv4_address == INADDR_NONE)
			{
				hostent* host = gethostbyname(_address.c_str());
				if (host)
				{
					ipv4_address = *((in_addr_t*)host->h_addr_list[0]);
				}
			}

			std::unique_lock<std::mutex> lock2(Mutex);
			ResolveStatus = (ipv4_address != INADDR_NONE) ? 2 : 3;
			ResolvedAddr.Addr = ipv4_address;
			ResolvedAddr.Port = htons(7777);
		};

		Thread = std::thread(threadMain);
	}
}

std::string UInternetLink::IpAddrToString(const IpAddr& Arg)
{
	uint32_t addr_long = (uint32_t)ntohl(Arg.Addr);
	return
		std::to_string((addr_long & 0xff000000) >> 24) + "." +
		std::to_string((addr_long & 0x00ff0000) >> 16) + "." +
		std::to_string((addr_long & 0x0000ff00) >> 8) + "." +
		std::to_string(addr_long & 0x000000ff) + ":" +
		std::to_string(ntohs(Arg.Port));
}

bool UInternetLink::StringToIpAddr(const std::string& Str, IpAddr& Addr)
{
	size_t portStart = Str.find(':');
	if (portStart == std::string::npos)
		portStart = Str.size();
	in_addr_t ipv4_address = inet_addr(Str.substr(0, portStart).c_str());
	Addr.Addr = ipv4_address;
	if (portStart == std::string::npos)
		Addr.Port = htons(7777);
	else
		Addr.Port = htons(std::atoi(Str.substr(portStart + 1).c_str()));
	return ipv4_address != INADDR_NONE && Addr.Port != 0 && Addr.Port != 0xffff;
}

/////////////////////////////////////////////////////////////////////////////

UTcpLink::UTcpLink(NameString name, UClass* base, ObjectFlags flags) : UInternetLink(name, base, flags)
{
	handle = socket(AF_INET, SOCK_STREAM, 0);

	if (handle != invalid_socket_value)
	{
#ifdef WIN32
		u_long nonblocking = 1;
		ioctlsocket(handle, FIONBIO, &nonblocking);
#else
		int nonblocking = 1;
		ioctl(handle, FIONBIO, &nonblocking);
#endif
	}
}

UTcpLink::~UTcpLink()
{
	if (handle != invalid_socket_value)
		closesocket(handle);
}

void UTcpLink::Tick(float elapsed, bool tickedFlag)
{
	UInternetLink::Tick(elapsed, tickedFlag);

	// event ReceivedText(string Text);
	// event ReceivedLine(string Line);
	// event ReceivedBinary(int Count, byte B[255]);

	// event Accepted();
	// event Opened();
	// event Closed();
}

int UTcpLink::BindPort(int Port, bool bUseNextAvailable)
{
	return 0;
}

bool UTcpLink::Listen()
{
	return false;
}

bool UTcpLink::Open(const IpAddr& Addr)
{
	return false;
}

bool UTcpLink::Close()
{
	return false;
}

bool UTcpLink::IsConnected()
{
	return false;
}

int UTcpLink::ReadBinary(int Count, uint8_t& B)
{
	return 0;
}

int UTcpLink::SendBinary(int Count, uint8_t B)
{
	return 0;
}

int UTcpLink::ReadText(std::string& Str)
{
	return 0;
}

int UTcpLink::SendText(const std::string& Str)
{
	return 0;
}

/////////////////////////////////////////////////////////////////////////////

UUdpLink::UUdpLink(NameString name, UClass* base, ObjectFlags flags) : UInternetLink(name, base, flags)
{
	handle = socket(AF_INET, SOCK_DGRAM, 0);
	if (handle != invalid_socket_value)
	{
#ifdef WIN32
		u_long nonblocking = 1;
		ioctlsocket(handle, FIONBIO, &nonblocking);
#else
		int nonblocking = 1;
		ioctl(handle, FIONBIO, &nonblocking);
#endif
	}
}

UUdpLink::~UUdpLink()
{
	if (handle != invalid_socket_value)
		closesocket(handle);
}

void UUdpLink::Tick(float elapsed, bool tickedFlag)
{
	UInternetLink::Tick(elapsed, tickedFlag);

	// int result = recvfrom(handle, (char*)data, size, 0, (sockaddr *)&addr, &addr_len);

	// event ReceivedText(string Text);
	// event ReceivedLine(string Line);
	// event ReceivedBinary(int Count, byte B[255]);
}

int UUdpLink::BindPort(int Port, bool bUseNextAvailable)
{
	sockaddr_in addr;
	memset(&addr, 0, sizeof(sockaddr_in));
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons(Port);

	int result = bind(handle, (const sockaddr*)&addr, sizeof(sockaddr_in));
	if (result == -1)
		return 0;

#ifdef WIN32
	int size = sizeof(sockaddr_in);
#else
	socklen_t size = sizeof(sockaddr_in);
#endif
	result = getsockname(handle, (sockaddr*)&addr, &size);
	if (result == -1)
		return 0;

	LocalIP.Addr = addr.sin_addr.s_addr;
	LocalIP.Port = addr.sin_port;

	return ntohs(addr.sin_port);
}

int UUdpLink::ReadBinary(IpAddr& Addr, int Count, uint8_t& B)
{
	return 0;
}

bool UUdpLink::SendBinary(const IpAddr& Addr, int Count, uint8_t B)
{
	return false;
}

int UUdpLink::ReadText(IpAddr& Addr, std::string& Str)
{
	return 0;
}

bool UUdpLink::SendText(const IpAddr& Addr, const std::string& Str)
{
	if (Str.size() > 0x7ffffff0)
		return false;

	sockaddr_in addr;
	memset(&addr, 0, sizeof(sockaddr_in));
	addr.sin_addr.s_addr = Addr.Addr;
	addr.sin_port = Addr.Port;

	std::string msg = Str;
	if (LinkMode() == MODE_Line)
		msg += "\r\n";

	int result = sendto(handle, msg.c_str(), (int)msg.size(), 0, (const sockaddr*)&addr, sizeof(sockaddr_in));
	return result != -1;
}
