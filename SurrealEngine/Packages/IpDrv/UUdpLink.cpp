
#include "Precomp.h"
#include "UUdpLink.h"
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

void UUdpLink::Tick(float elapsed)
{
	UInternetLink::Tick(elapsed);

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
