
#include "Precomp.h"
#include "DebuggerApp.h"
#include "Utils/UTF16.h"
#include <iostream>
#include <vector>
#ifdef WIN32
#include <CommCtrl.h>
#endif

#ifdef WIN32

#pragma comment(linker, "\"/manifestdependency:type='Win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "Ws2_32.lib")

// Attempt to restore old console modes when process exits
struct ApplyConsoleMode
{
	ApplyConsoleMode()
	{
		oldconsolecp = GetConsoleCP();
		oldconsoleoutputcp = GetConsoleOutputCP();
		SetConsoleCP(CP_UTF8);
		SetConsoleOutputCP(CP_UTF8);

		stdoutput = GetStdHandle(STD_OUTPUT_HANDLE);
		if (stdoutput != INVALID_HANDLE_VALUE)
		{
			oldoutputmodeset = GetConsoleMode(stdoutput, &oldoutputmode);
			SetConsoleMode(stdoutput, ENABLE_VIRTUAL_TERMINAL_PROCESSING | ENABLE_PROCESSED_OUTPUT | ENABLE_WRAP_AT_EOL_OUTPUT | DISABLE_NEWLINE_AUTO_RETURN);
		}

		stdinput = GetStdHandle(STD_INPUT_HANDLE);
		if (stdinput != INVALID_HANDLE_VALUE)
		{
			oldinputmodeset = GetConsoleMode(stdoutput, &oldinputmode);
			SetConsoleMode(stdinput, ENABLE_VIRTUAL_TERMINAL_INPUT | ENABLE_PROCESSED_INPUT);
		}
	}
	~ApplyConsoleMode()
	{
		if (oldoutputmodeset)
			SetConsoleMode(stdoutput, oldoutputmode);

		if (oldinputmodeset)
			SetConsoleMode(stdinput, oldinputmode);

		SetConsoleCP(oldconsolecp);
		SetConsoleOutputCP(oldconsoleoutputcp);
	}

	UINT oldconsolecp = 0;
	UINT oldconsoleoutputcp = 0;
	BOOL oldoutputmodeset = FALSE;
	BOOL oldinputmodeset = FALSE;
	DWORD oldoutputmode = 0;
	DWORD oldinputmode = 0;
	HANDLE stdoutput = INVALID_HANDLE_VALUE;
	HANDLE stdinput = INVALID_HANDLE_VALUE;
};

int wmain(int argc, wchar_t* argv[])
{
	ApplyConsoleMode applyConsoleMode;

	try
	{
		Array<std::string> args;
		for (int i = 1; i < argc; i++)
			args.push_back(from_utf16(argv[i]));

		//SetProcessDPIAware();
		SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE);
		InitCommonControls();

		WORD winsock_version = MAKEWORD(2, 2);
		WSADATA wsaData;
		int err = WSAStartup(winsock_version, &wsaData);
		if (err != 0)
			throw std::runtime_error("Failed to initialize winsockets");

		DebuggerApp app;
		return app.Main(std::move(args));
	}
	catch (const std::exception& e)
	{
		std::cout << e.what() << std::endl;
		return 1;
	}
}

#else

int main(int argc, char** argv)
{
	try
	{
		Array<std::string> args;
		for (int i = 1; i < argc; i++)
			args.push_back(argv[i]);

		DebuggerApp app;
		return app.Main(std::move(args));
	}
	catch (const std::exception& e)
	{
		std::cout << e.what() << std::endl;
		return 1;
	}
}

#endif
