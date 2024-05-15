#pragma once

#include <mutex>
#include <string>

class Exception
{
public:
	static bool Init();
	static bool Exit();

	[[noreturn]] static void Throw(const std::string& text);

private:
	static int CaptureStackFrames(std::ostringstream& sstream, int maxframes);

	static std::mutex mutex;
	static bool bInited;
	static bool bHasSymbols;
	static void* PlatformData;
};