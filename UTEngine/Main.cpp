
#include "Precomp.h"
#include "Engine.h"
#include "UTF16.h"
#include "VM/Frame.h"
#include <iostream>
#ifdef WIN32
#include <CommCtrl.h>
#endif

void appMain()
{
	Engine engine;
	engine.Run();
}

#ifdef WIN32

#pragma comment(linker, "\"/manifestdependency:type='Win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#pragma comment(lib, "comctl32.lib")

int WINAPI wWinMain(HINSTANCE, HINSTANCE, PWSTR, int)
{
#ifndef _DEBUG
	try
#endif
	{
		SetProcessDPIAware();
		InitCommonControls();
		appMain();
		return 0;
	}
#ifndef _DEBUG
	catch (const std::exception& e)
	{
		MessageBox(0, to_utf16(e.what()).c_str(), to_utf16("Unhandled Exception").c_str(), MB_OK | MB_ICONEXCLAMATION);
		return 1;
	}
#endif
}

#else

int main()
{
	try
	{
		appMain();
		return 0;
	}
	catch (const std::exception& e)
	{
		std::cout << "Unhandled exception: " << e.what() << std::endl;
		return 1;
	}
}

#endif
