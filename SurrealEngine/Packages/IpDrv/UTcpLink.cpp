
#include "Precomp.h"
#include "UTcpLink.h"
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

void UTcpLink::Tick(float elapsed)
{
	UInternetLink::Tick(elapsed);

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
