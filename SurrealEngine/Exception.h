#pragma once

#include <string>

class Exception
{
public:
	static bool Init();
	static bool Exit();

	[[noreturn]] static void Throw(const std::string& text);

private:
	static int CaptureStackFrames(std::ostringstream& sstream, int maxframes);

	static bool bInited;
	static bool bHasSymbols;
	static void* PlatformData;
};